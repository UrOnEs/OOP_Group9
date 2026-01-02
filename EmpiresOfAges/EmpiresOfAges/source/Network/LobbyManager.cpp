#include "Network/LobbyManager.h"
#include "Network/NetworkManager.h"
#include "Network/NetServer.h"
#include "Network/NetClient.h"
#include <SFML/Network/IpAddress.hpp> 
#include <iostream>
#include <algorithm>
#include <stdexcept>

LobbyManager::LobbyManager(NetworkManager* netManager, bool isHost)
    : m_netManager(netManager), m_isHost(isHost), m_selfId(0) {}

void LobbyManager::start(uint64_t selfId, const std::string& name) {
    m_selfId = selfId;
    // Default color 0 (Red)
    addPlayer(selfId, name, false, 0);

    if (!m_isHost) {
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(LobbyCommand::JoinRequest) << name;
        if (m_netManager && m_netManager->client()) {
            m_netManager->client()->sendReliable(pkt);
        }
    }
}

void LobbyManager::changeColor(int colorIndex) {
    if (m_isHost) {
        // If host, change directly and sync
        setColor(m_selfId, colorIndex);
        syncLobbyToClients();
    }
    else {
        // If client, send request to server
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(LobbyCommand::ChangeColor) << colorIndex;
        if (m_netManager && m_netManager->client()) {
            m_netManager->client()->sendReliable(pkt);
        }
    }
}

void LobbyManager::closeLobby() {
    if (m_isHost && m_netManager->server()) {
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(LobbyCommand::LobbyClosed);
        m_netManager->server()->sendToAllReliable(pkt);
    }
}

void LobbyManager::toggleReady(bool isReady) {
    if (m_isHost) {
        setReady(m_selfId, isReady);
        syncLobbyToClients();
    }
    else {
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(LobbyCommand::ToggleReady) << isReady;
        if (m_netManager && m_netManager->client()) m_netManager->client()->sendReliable(pkt);
    }
}

void LobbyManager::startGame() {
    if (!m_isHost || !canStartGame() || m_isGameStarted) return;
    m_isGameStarted = true;

    m_gameSeed = static_cast<unsigned int>(std::time(nullptr));

    sf::Packet pkt;
    pkt << static_cast<sf::Int32>(LobbyCommand::StartGameSignal) << m_gameSeed;
    if (m_netManager && m_netManager->server()) {
        m_netManager->server()->sendToAllReliable(pkt);
        if (m_onGameStart) m_onGameStart();
    }
}

bool LobbyManager::canStartGame() const {
    if (m_players.size() < 1) return false;
    for (const auto& p : m_players) if (!p.ready) return false;
    return true;
}

void LobbyManager::setOnPlayerChange(PlayerChangeCallback cb) { m_onChange = std::move(cb); }
void LobbyManager::setOnGameStart(GameStartCallback cb) { m_onGameStart = std::move(cb); }

void LobbyManager::handleIncomingPacket(uint64_t senderId, sf::Packet& pkt) {
    sf::Int32 cmdInt;
    if (!(pkt >> cmdInt)) return;
    LobbyCommand cmd = static_cast<LobbyCommand>(cmdInt);

    if (m_isHost) {
        if (cmd == LobbyCommand::JoinRequest) processJoinRequest(senderId, pkt);
        else if (cmd == LobbyCommand::ToggleReady) processToggleReady(senderId, pkt);
        else if (cmd == LobbyCommand::LeaveLobby) removePlayer(senderId);
        else if (cmd == LobbyCommand::ChangeColor) processChangeColor(senderId, pkt);
    }
    else {
        if (cmd == LobbyCommand::LobbyStateSync) {
            sf::Uint64 selfIdInPkt;
            if (!(pkt >> selfIdInPkt)) return;
            m_selfId = selfIdInPkt;

            m_players.clear();
            sf::Uint32 playerCount;
            if (pkt >> playerCount) {
                for (sf::Uint32 i = 0; i < playerCount; ++i) {
                    PlayerInfo p;
                    sf::Uint64 id;
                    std::string name;
                    bool ready;
                    int color;

                    if (pkt >> id >> name >> ready >> color) {
                        p.id = id;
                        p.name = name;
                        p.ready = ready;
                        p.colorIndex = color;
                        m_players.push_back(p);
                    }
                }
                if (m_onChange) m_onChange();
            }
        }
        else if (cmd == LobbyCommand::StartGameSignal) {
            unsigned int seed = 0;
            if (pkt >> seed) {
                m_gameSeed = seed;
                if (m_onGameStart) m_onGameStart();
            }
        }
        else if (cmd == LobbyCommand::LobbyClosed) {
            if (m_netManager && m_netManager->client()) m_netManager->client()->disconnect();
        }
    }
}

void LobbyManager::processJoinRequest(uint64_t senderId, sf::Packet& pkt) {
    std::string playerName;
    if (!(pkt >> playerName)) return;

    // Assign default color
    int defaultColor = static_cast<int>(m_players.size()) % 4;
    addPlayer(senderId, playerName, false, defaultColor);
    syncLobbyToClients();

    // If game already started, send the seed to the late joiner
    if (m_isGameStarted) {
        sf::Packet startPkt;
        startPkt << static_cast<sf::Int32>(LobbyCommand::StartGameSignal) << m_gameSeed;

        if (m_netManager && m_netManager->server()) {
            m_netManager->server()->sendToReliable(senderId, startPkt);
        }
    }
}

void LobbyManager::processToggleReady(uint64_t senderId, sf::Packet& pkt) {
    bool isReady;
    if (!(pkt >> isReady)) return;
    setReady(senderId, isReady);
    syncLobbyToClients();
}

void LobbyManager::processChangeColor(uint64_t senderId, sf::Packet& pkt) {
    int newColor;
    if (!(pkt >> newColor)) return;
    setColor(senderId, newColor);
    syncLobbyToClients();
}

void LobbyManager::syncLobbyToClients() {
    if (!m_isHost || !m_netManager->server()) return;

    for (const auto& client : m_players) {
        if (client.id == m_selfId) continue;

        sf::Packet syncPkt;
        syncPkt << static_cast<sf::Int32>(LobbyCommand::LobbyStateSync);
        syncPkt << static_cast<sf::Uint64>(client.id);
        syncPkt << static_cast<sf::Uint32>(m_players.size());

        for (const auto& p : m_players) {
            syncPkt << static_cast<sf::Uint64>(p.id)
                << p.name
                << p.ready
                << static_cast<sf::Int32>(p.colorIndex);
        }
        m_netManager->server()->sendToReliable(client.id, syncPkt);
    }
    if (m_onChange) m_onChange();
}

void LobbyManager::addPlayer(uint64_t id, const std::string& name, bool ready, int colorIndex) {
    auto it = std::find_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; });
    if (it == m_players.end()) {
        m_players.push_back({ id, name, ready, colorIndex });
    }
}

void LobbyManager::removePlayer(uint64_t id) {
    auto it = std::remove_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; });
    if (it != m_players.end()) {
        m_players.erase(it, m_players.end());
        if (m_isHost) syncLobbyToClients();
        if (m_onChange) m_onChange();
    }
}

void LobbyManager::setReady(uint64_t id, bool ready) {
    auto it = std::find_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; });
    if (it != m_players.end()) it->ready = ready;
}

void LobbyManager::setColor(uint64_t id, int colorIndex) {
    auto it = std::find_if(m_players.begin(), m_players.end(),
        [id](const PlayerInfo& p) { return p.id == id; });
    if (it != m_players.end()) {
        it->colorIndex = colorIndex;
    }
}