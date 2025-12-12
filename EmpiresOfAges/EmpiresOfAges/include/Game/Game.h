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
    sf::View camera; // Haritada gezmek için kamera þart!

    // --- YÖNETÝCÝLER (MANAGERS) ---
    // Bu sýnýflarý Game'in içinde tutmalýsýn ki oyun boyunca yaþasýnlar
    NetworkManager networkManager;
    std::unique_ptr<LobbyManager> lobbyManager; // NetworkManager baþlatýlýnca oluþturulacak
    MapManager mapManager;
    UIManager uiManager;
    GameStateManager stateManager;

    // --- OYUN NESNELERÝ ---
    Player localPlayer; // Bizim yönettiðimiz oyuncu
    HUD hud; // Arayüz göstergesi

    // --- YARDIMCI FONKSÝYONLAR ---
    void initNetwork(); // Aðý baþlatma iþlemleri
    void initUI();      // Butonlarý ve panelleri oluþturma
    void handleInput(float dt); // Kamera ve týklama kontrolleri
};

