#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

class NetworkManager;

// 1. GÜNCELLEME: ChangeColor komutu eklendi
enum class LobbyCommand : sf::Int32 {
    JoinRequest = 0,
    LobbyStateSync = 1,
    ToggleReady = 2,
    StartGameSignal = 3,
    LeaveLobby = 4,
    LobbyClosed = 5,
    ChangeColor = 6  // <--- YENİ: Renk Değiştirme Komutu
};

// 2. GÜNCELLEME: Renk indeksi eklendi
struct PlayerInfo {
    uint64_t id;
    std::string name;
    bool ready;
    int colorIndex; // 0: Kırmızı, 1: Mavi, 2: Yeşil...
};

class LobbyManager {
public:
    LobbyManager(NetworkManager* netManager, bool isHost);

    void start(uint64_t selfId, const std::string& name);
    void toggleReady(bool isReady);

    // 3. GÜNCELLEME: Renk değiştirme fonksiyonu
    void changeColor(int colorIndex);

    void startGame();
    void closeLobby();
    void handleIncomingPacket(uint64_t senderId, sf::Packet& pkt);

    uint64_t selfId() const { return m_selfId; }
    const std::vector<PlayerInfo>& players() const { return m_players; }
    bool canStartGame() const;

    using PlayerChangeCallback = std::function<void()>;
    using GameStartCallback = std::function<void()>;
    void setOnPlayerChange(PlayerChangeCallback cb);
    void setOnGameStart(GameStartCallback cb);

    void removePlayer(uint64_t id);
    void syncLobbyToClients();

private:
    void processJoinRequest(uint64_t senderId, sf::Packet& pkt);
    void processToggleReady(uint64_t senderId, sf::Packet& pkt);

    // 4. GÜNCELLEME: Sunucu tarafında renk işleme
    void processChangeColor(uint64_t senderId, sf::Packet& pkt);

    void addPlayer(uint64_t id, const std::string& name, bool ready = false, int colorIndex = 0);
    void setReady(uint64_t id, bool ready);

    // Yardımcı: Sadece rengi set etme
    void setColor(uint64_t id, int colorIndex);

    NetworkManager* m_netManager;
    bool m_isHost;
    uint64_t m_selfId;
    std::vector<PlayerInfo> m_players;

    PlayerChangeCallback m_onChange;
    GameStartCallback m_onGameStart;
    bool m_isGameStarted = false;
};