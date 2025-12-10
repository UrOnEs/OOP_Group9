#pragma once
// LobbyManager.h

#pragma once

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

// �leri Bildirim (Forward Declarations)
class NetworkManager;

// Lobi �leti�im Komutlar�
enum class LobbyCommand : sf::Int32 {
    JoinRequest,
    LobbyStateSync,
    ToggleReady,
    StartGameSignal,
    // ... di�er komutlar
};

// Oyuncu Bilgisi Yap�s�
struct PlayerInfo {
    uint64_t id;
    std::string name;
    bool ready;
    // ... di�er lobi bilgileri
};

class LobbyManager {
public:
    // Yap�c�
    LobbyManager(NetworkManager* netManager, bool isHost);

    // A� ve Lobi Ba�latma
    void start(uint64_t selfId, const std::string& name);

    // D�� d�nya ile etkile�im
    void toggleReady(bool isReady);
    void startGame();

    // Gelen Paket ��leme
    void handleIncomingPacket(uint64_t senderId, sf::Packet& pkt);

    // Getter'lar (Hata E0135 ve C2039 ��z�m� i�in KR�T�K)
    uint64_t selfId() const { return m_selfId; } // <-- selfId eklendi
    const std::vector<PlayerInfo>& players() const { return m_players; }
    bool canStartGame() const;

    // Callback Ayarlar�
    using PlayerChangeCallback = std::function<void()>;
    using GameStartCallback = std::function<void()>;
    void setOnPlayerChange(PlayerChangeCallback cb);
    void setOnGameStart(GameStartCallback cb);

private:
    // Lobi Y�netimi (Sunucu Taraf�)
    void processJoinRequest(uint64_t senderId, sf::Packet& pkt);
    void processToggleReady(uint64_t senderId, sf::Packet& pkt);
    void syncLobbyToClients(); // T�m istemcilere lobi durumunu g�nder

    // Yard�mc� Metotlar
    void addPlayer(uint64_t id, const std::string& name, bool ready = false);
    void removePlayer(uint64_t id);
    void setReady(uint64_t id, bool ready);

    // �ye De�i�kenleri
    NetworkManager* m_netManager;
    bool m_isHost;
    uint64_t m_selfId; // Kendi ID'miz (Host i�in 1, Client i�in Sunucudan gelen ID)
    std::vector<PlayerInfo> m_players;

    // Callback'ler
    PlayerChangeCallback m_onChange;
    GameStartCallback m_onGameStart;
    bool m_isGameStarted = false;
};