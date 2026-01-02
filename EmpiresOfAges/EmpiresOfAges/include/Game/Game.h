#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio/Music.hpp>
#include <vector>
#include <memory>

// --- MANAGERS ---
#include "Network/NetworkManager.h"
#include "Network/LobbyManager.h"
#include "Map/MapManager.h"
#include "UI/UIManager.h"
#include "UI/HUD.h"
#include "Game/GameStateManager.h"
#include "Map/FogOfWar.h"

// --- ENTITIES ---
#include "Game/Player.h"
#include "Entity System/Entity Type/types.h" 

/**
 * @brief The main engine class that manages the game loop, rendering, and logic.
 * Orchestrates communication between the Network, Map, UI, and Player systems.
 */
class Game {
public:
    /**
     * @brief Constructs the Game instance.
     * @param isHost True if this instance is hosting the multiplayer session.
     * @param serverIp The IP address of the server (if client).
     * @param playerIndex The index/ID of the local player (determines spawn point and color).
     * @param totalPlayerCount Total number of players expected in the match.
     */
    Game(bool isHost, std::string serverIp, int playerIndex, int totalPlayerCount);

    /**
     * @brief Starts the main game loop (update/render).
     */
    void run();

    /**
     * @brief Initializes the match environment based on a synchronized seed.
     * Generates the map, places starting units, and sets up the HUD.
     * @param seed The random seed for procedural generation.
     */
    void startMatch(unsigned int seed);

    /**
     * @brief Displays a warning message on the screen (e.g., "Not Enough Resources").
     */
    void showWarning(const std::string& message);
    void drawWarning(sf::RenderWindow& window);

    // --- Network Commands ---

    /**
     * @brief Sends a network packet to train a unit.
     */
    void sendTrainCommand(int gridX, int gridY, int unitTypeID);

    /**
     * @brief Sends a network packet to construct a building.
     */
    void sendBuildCommand(int gridX, int gridY, int buildTypeID);

private:
    void processEvents();
    void update(float dt);
    void render();

    std::string warningMsg = "";
    sf::Clock warningClock;
    bool isWarningActive = false;

    // --- Core Components ---
    sf::RenderWindow window;
    sf::View camera;

    // --- Managers ---
    NetworkManager networkManager;
    std::unique_ptr<LobbyManager> lobbyManager;
    MapManager mapManager;
    UIManager uiManager;
    GameStateManager stateManager;

    // --- Game Objects ---
    Player localPlayer;
    Player enemyPlayer;
    HUD hud;

    // --- Audio ---
    sf::Music bgMusic;

    // --- Build System Variables ---
    bool isInBuildMode = false;          ///< True if player is placing a building.
    BuildTypes pendingBuildingType;      ///< The type of building currently being placed.
    sf::Sprite ghostBuildingSprite;      ///< Translucent preview of the building under mouse.
    sf::RectangleShape ghostGridRect;    ///< Grid visualizer for placement.

    // --- Helper Functions ---
    void initNetwork();
    void initUI();
    void handleInput(float dt);

    void handleKeyboardInput(const sf::Event& event);
    void handleMouseInput(const sf::Event& event);

    void onLeftClick(const sf::Vector2f& worldPos, const sf::Vector2i& pixelPos);
    void onRightClick(const sf::Vector2f& worldPos);

    void enterBuildMode(BuildTypes type, const std::string& textureName);
    void cancelBuildMode();

    bool isSelecting = false;        ///< Is the player currently dragging a selection box?
    sf::Vector2f selectionStartPos;  ///< World position where selection started.
    sf::RectangleShape selectionBox; ///< Visual representation of the selection area.

    // --- Fog of War ---
    std::unique_ptr<FogOfWar> m_fogOfWar;

    float gameDuration = 0.0f;
    bool m_isHost;
    std::string m_serverIp;

    bool isDevMode = false;

    int m_playerIndex;

    bool m_matchStarted = false;
    bool m_launchScheduled = false;

    int m_totalPlayerCount;
    int m_connectedClientCount;

    float m_startGameTimer = -1.0f;
};