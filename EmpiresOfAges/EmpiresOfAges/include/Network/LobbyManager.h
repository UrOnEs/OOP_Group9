#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/IpAddress.hpp>

class NetworkManager;

/**
 * @brief Commands specific to lobby management.
 */
enum class LobbyCommand : sf::Int32 {
    JoinRequest = 0,
    LobbyStateSync = 1,
    ToggleReady = 2,
    StartGameSignal = 3,
    LeaveLobby = 4,
    LobbyClosed = 5,
    ChangeColor = 6
};

/**
 * @brief Represents a player in the lobby.
 */
struct PlayerInfo {
    uint64_t id;
    std::string name;
    bool ready;
    int colorIndex; // 0: Red, 1: Blue, 2: Green...
};

/**
 * @brief Manages the pre-game lobby, including player synchronization, readiness, and colors.
 */
class LobbyManager {
public:
    LobbyManager(NetworkManager* netManager, bool isHost);

    void start(uint64_t selfId, const std::string& name);
    void toggleReady(bool isReady);

    /**
     * @brief Requests a color change for the local player.
     */
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

    unsigned int getGameSeed() const { return m_gameSeed; }

private:
    void processJoinRequest(uint64_t senderId, sf::Packet& pkt);
    void processToggleReady(uint64_t senderId, sf::Packet& pkt);
    void processChangeColor(uint64_t senderId, sf::Packet& pkt);

    void addPlayer(uint64_t id, const std::string& name, bool ready = false, int colorIndex = 0);
    void setReady(uint64_t id, bool ready);
    void setColor(uint64_t id, int colorIndex);

    NetworkManager* m_netManager;
    bool m_isHost;
    uint64_t m_selfId;
    std::vector<PlayerInfo> m_players;

    PlayerChangeCallback m_onChange;
    GameStartCallback m_onGameStart;
    bool m_isGameStarted = false;

    unsigned int m_gameSeed = 0;
};