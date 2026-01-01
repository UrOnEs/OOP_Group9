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


class Game {
public:
    Game(bool isHost, std::string serverIp, int playerIndex, int totalPlayerCount);
    void run();
    void startMatch(unsigned int seed);


    void showWarning(const std::string& message);
    void drawWarning(sf::RenderWindow& window);

    void sendTrainCommand(int gridX, int gridY, int unitTypeID);

    void sendBuildCommand(int gridX, int gridY, int buildTypeID);

private:
    void processEvents();
    void update(float dt);
    void render();
    std::string warningMsg = "";
    sf::Clock warningClock;
    bool isWarningActive = false;

    // --- TEMEL BÝLEÞENLER ---
    sf::RenderWindow window;
    sf::View camera;

    // --- YÖNETÝCÝLER (MANAGERS) ---
    NetworkManager networkManager;
    std::unique_ptr<LobbyManager> lobbyManager;
    MapManager mapManager;
    UIManager uiManager;
    GameStateManager stateManager;


    // --- OYUN NESNELERÝ ---
    Player localPlayer;
    Player enemyPlayer;
    HUD hud;

    // ----- Sesler ----------
    sf::Music bgMusic;



    // --- ÝNÞAAT SÝSTEMÝ DEÐÝÞKENLERÝ (YENÝ) ---
    bool isInBuildMode = false;          // Þu an bina yerleþtirmeye çalýþýyor muyuz?
    BuildTypes pendingBuildingType;      // Hangi binayý yerleþtireceðiz?
    sf::Sprite ghostBuildingSprite;      // Mouse ucundaki yarý saydam bina görseli
    sf::RectangleShape ghostGridRect;    // Izgarayý göstermek için kare (Opsiyonel ama þýk durur)

    // --- YARDIMCI FONKSÝYONLAR ---
    void initNetwork();
    void initUI();
    void handleInput(float dt);


    void handleKeyboardInput(const sf::Event& event);
    void handleMouseInput(const sf::Event& event);

    void onLeftClick(const sf::Vector2f& worldPos, const sf::Vector2i& pixelPos);
    void onRightClick(const sf::Vector2f& worldPos);

    // Ýnþaat modunu açýp kapatan yardýmcý fonksiyon
    void enterBuildMode(BuildTypes type, const std::string& textureName);
    void cancelBuildMode();

    bool isSelecting = false;        // Mouse basýlý mý?
    sf::Vector2f selectionStartPos;  // Ýlk týkladýðýmýz yer (Dünya koordinatý)
    sf::RectangleShape selectionBox; // çizilecek yeþil kutu

    //--- SAVAÞ SÝSÝ ---
    std::unique_ptr<FogOfWar> m_fogOfWar;

    //Oyun ne kadar süredir aktif

    float gameDuration = 0.0f;
    bool m_isHost;
    std::string m_serverIp;

    bool isDevMode = false;

    int m_playerIndex;

    bool m_matchStarted = false;       // Oyunun baþlayýp baþlamadýðýný kontrol eder
    bool m_launchScheduled = false;

    int m_totalPlayerCount;     // Oyunda olmasý gereken toplam kiþi sayýsý
    int m_connectedClientCount;

    float m_startGameTimer = -1.0f;
};