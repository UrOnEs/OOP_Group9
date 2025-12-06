// LobbyManager.cpp

#include "LobbyManager.h"
#include "NetworkManager.h"
#include "NetServer.h"
#include "NetClient.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>

// --- Yap�c� ---
LobbyManager::LobbyManager(NetworkManager* netManager, bool isHost)
    : m_netManager(netManager), m_isHost(isHost), m_selfId(0) {} // <-- m_selfId ba�lat�ld�

// --- D�� Fonksiyonlar ---

void LobbyManager::start(uint64_t selfId, const std::string& name) {
    m_selfId = selfId;
    addPlayer(selfId, name, false);

    if (!m_isHost) {
        // �stemci: JoinRequest g�nder
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(LobbyCommand::JoinRequest)
            << name;

        if (m_netManager && m_netManager->client()) {
            m_netManager->client()->send(pkt);
            std::cout << "[CLIENT] Join Request gonderildi: " << name << std::endl;
        }
        else {
            throw std::runtime_error("Client bagli degil.");
        }
    }
}

void LobbyManager::toggleReady(bool isReady) {
    if (m_isHost) {
        // Host: Kendi durumunu yerelde de�i�tir ve herkese senkronize et
        setReady(m_selfId, isReady);
        syncLobbyToClients();
    }
    else {
        // �stemci: Sunucuya Ready durumunu de�i�tirme iste�i g�nder
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(LobbyCommand::ToggleReady)
            << isReady;

        if (m_netManager && m_netManager->client()) {
            m_netManager->client()->send(pkt);
        }
    }
}

void LobbyManager::startGame() {
    if (!m_isHost || !canStartGame() || m_isGameStarted) return;

    // Bayra�� hemen true yap�yoruz
    m_isGameStarted = true;

    sf::Packet pkt;
    pkt << static_cast<sf::Int32>(LobbyCommand::StartGameSignal);

    if (m_netManager && m_netManager->server()) {
        m_netManager->server()->sendToAll(pkt);
        if (m_onGameStart) m_onGameStart();
    }
}

bool LobbyManager::canStartGame() const {
    if (m_players.size() < 2) return false;
    for (const auto& p : m_players) {
        if (!p.ready) return false;
    }
    return true;
}

// --- Callback Ayarlar� ---

void LobbyManager::setOnPlayerChange(PlayerChangeCallback cb) {
    m_onChange = std::move(cb);
}

void LobbyManager::setOnGameStart(GameStartCallback cb) {
    m_onGameStart = std::move(cb);
}

// --- Paket ��leme ---

void LobbyManager::handleIncomingPacket(uint64_t senderId, sf::Packet& pkt) {
    sf::Int32 cmdInt;
    if (!(pkt >> cmdInt)) return;

    LobbyCommand cmd = static_cast<LobbyCommand>(cmdInt);

    if (m_isHost) {
        // Sunucu Taraf� ��leme
        if (cmd == LobbyCommand::JoinRequest) {
            processJoinRequest(senderId, pkt);
        }
        else if (cmd == LobbyCommand::ToggleReady) {
            processToggleReady(senderId, pkt);
        }

    }
    else {
        // �stemci Taraf� ��leme
        if (cmd == LobbyCommand::LobbyStateSync) {
            // Lobi durumunu senkronize et
            sf::Uint64 selfIdInPkt;
            if (!(pkt >> selfIdInPkt)) return;
            m_selfId = selfIdInPkt; // Sunucudan gelen kendi ID'mizi kaydet (KR�T�K)

            m_players.clear();
            sf::Uint32 playerCount;

            if (pkt >> playerCount) {
                for (sf::Uint32 i = 0; i < playerCount; ++i) {
                    PlayerInfo p;
                    sf::Uint64 id;
                    std::string name;
                    bool ready;

                    if (pkt >> id >> name >> ready) {
                        p.id = id;
                        p.name = name;
                        p.ready = ready;
                        m_players.push_back(p);
                    }
                }
                if (m_onChange) m_onChange();
            }
        }
        else if (cmd == LobbyCommand::StartGameSignal) {
            if (m_onGameStart) m_onGameStart();
        }
    }
}

// --- Sunucu Taraf� Mant�k ---

void LobbyManager::processJoinRequest(uint64_t senderId, sf::Packet& pkt) {
    std::string playerName;
    if (!(pkt >> playerName)) return;

    // Oyuncuyu listeye ekle
    addPlayer(senderId, playerName, false);
    std::cout << "[LobbyManager-Host] Yeni katilim istegi: ID=" << senderId << ", Ad=" << playerName << std::endl;

    // T�m istemcilere (yeni gelen dahil) lobi durumunu g�nder
    syncLobbyToClients();
}

void LobbyManager::processToggleReady(uint64_t senderId, sf::Packet& pkt) {
    bool isReady;
    if (!(pkt >> isReady)) return;

    // Oyuncunun durumunu de�i�tir
    setReady(senderId, isReady);
    std::cout << "[LobbyManager-Host] ID " << senderId << " hazir durumu: " << (isReady ? "EVET" : "HAYIR") << std::endl;

    // T�m istemcilere lobi durumunu senkronize et
    syncLobbyToClients();
}

void LobbyManager::syncLobbyToClients() {
    if (!m_isHost || !m_netManager->server()) return;

    for (const auto& client : m_players) {
        if (client.id == m_selfId) continue; // Host'a g�nderme

        sf::Packet syncPkt;
        syncPkt << static_cast<sf::Int32>(LobbyCommand::LobbyStateSync);

        syncPkt << static_cast<sf::Uint64>(client.id); // �stemcinin kendi ID'si

        syncPkt << static_cast<sf::Uint32>(m_players.size());

        for (const auto& p : m_players) {
            syncPkt << static_cast<sf::Uint64>(p.id)
                << p.name
                << p.ready;
        }

        if (m_netManager->server()->sendTo(client.id, syncPkt)) {
            // Ba�ar�l� g�nderim
        }
        else {
            // Hata logu eklenebilir
        }
    }

    // Host'un lobi durumu da de�i�ti�i i�in callback'i tetikle
    if (m_onChange) m_onChange();
}

// --- Yard�mc� Metotlar ---

void LobbyManager::addPlayer(uint64_t id, const std::string& name, bool ready) {
    auto it = std::find_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; });

    if (it == m_players.end()) {
        m_players.push_back({ id, name, ready });
        // if (m_onChange && m_isHost) m_onChange(); 
    }
}

void LobbyManager::removePlayer(uint64_t id) {
    m_players.erase(std::remove_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; }),
        m_players.end());
}

void LobbyManager::setReady(uint64_t id, bool ready) {
    auto it = std::find_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; });

    if (it != m_players.end() && it->ready != ready) {
        it->ready = ready;
        if (m_isHost) {
            // Sunucu, durumu de�i�tirdikten sonra manuel olarak callback'i tetikler.
        }
    }
}