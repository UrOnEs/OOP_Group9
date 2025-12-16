#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

// --- MANAGERS ---
#include "Network/NetworkManager.h"
#include "Network/LobbyManager.h"
#include "Map/MapManager.h"
#include "UI/UIManager.h"
#include "UI/HUD.h"
#include "Game/GameStateManager.h"

// --- ENTITIES ---
#include "Game/Player.h"
#include "Entity System/Entity Type/types.h" 

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

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
    HUD hud;

    // --- ÝNÞAAT SÝSTEMÝ DEÐÝÞKENLERÝ (YENÝ) ---
    bool isInBuildMode = false;          // Þu an bina yerleþtirmeye çalýþýyor muyuz?
    BuildTypes pendingBuildingType;      // Hangi binayý yerleþtireceðiz?
    sf::Sprite ghostBuildingSprite;      // Mouse ucundaki yarý saydam bina görseli
    sf::RectangleShape ghostGridRect;    // Izgarayý göstermek için kare (Opsiyonel ama þýk durur)

    // --- YARDIMCI FONKSÝYONLAR ---
    void initNetwork();
    void initUI();
    void handleInput(float dt);

    // Ýnþaat modunu açýp kapatan yardýmcý fonksiyon
    void enterBuildMode(BuildTypes type, const std::string& textureName);
    void cancelBuildMode();
};