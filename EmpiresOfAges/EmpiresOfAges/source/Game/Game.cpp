#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include <iostream>

Game::Game()
    : mapManager(50, 50, 32) // 50x50 harita, 32px tile
{
    window.create(sf::VideoMode(1280, 720), "Empires of Ages - RTS");
    window.setFramerateLimit(60);

    // Kamerayý baþlat
    camera.setSize(1280, 720);
    camera.setCenter(1280 / 2, 720 / 2);

    initUI();
    initNetwork();
    stateManager.setState(GameState::Playing);

    // Haritayý oluþtur
    mapManager.initialize();
}

void Game::initNetwork() {
    // 1. Loglama sistemini baðla
    networkManager.setLogger([](const std::string& msg) {
        std::cout << "[NETWORK]: " << msg << std::endl;
        });

    // 2. Lobby Manager'ý oluþtur
    // Server mý Client mý olduðunu menüden seçmen gerekecek ama þimdilik manuel test:
    // networkManager.startServer(54000); 

    lobbyManager = std::make_unique<LobbyManager>(&networkManager, false);

    // 3. Oyun Baþlama Sinyalini Baðla (LobbyManager -> Game)
    lobbyManager->setOnGameStart([this]() {
        std::cout << "OYUN BASLIYOR!\n";
        stateManager.setState(GameState::Playing);
        });
}

void Game::initUI() {
    // Örnek: Bir panel ekle (UIManager.h kullanýlarak)
    // uiManager.addPanel(...);
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time dt = clock.restart();
        processEvents();
        update(dt.asSeconds());
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        // UI Olaylarýný iþle (Týklama vs.)
        uiManager.handleEvent(event);

        if (event.type == sf::Event::Closed)
            window.close();

        // Harita üzerindeki týklamalar (Bina yapma, asker seçme)
        // MapManager.h içindeki handleInput çaðrýlmalý ama parametreleri düzeltmen lazým
        // mapManager.handleInput(window, camera, localPlayer.playerResources); 
    }
}

void Game::update(float dt) {
    // 1. AÐ GÜNCELLEMESÝ (Paket alma/gönderme)
    networkManager.update(dt);

    // 2. STATE KONTROLÜ
    if (stateManager.getState() == GameState::Playing) {

        // --- KAMERA HAREKETÝ ---
        handleInput(dt);

        // --- SÝSTEMLERÝN ÇALIÞTIÐI YER (Core Mechanics) ---

        // A) Tüm binalarý güncelle (Üretim, Kaynak)
        // MapManager içindeki binalarý ProductionSystem ile kontrol etmemiz lazým.
        // Bunun için MapManager'a "getBuildings" metodu ekleyip burada döngüye sokmalýsýn.
        // Veya MapManager::updateBuildings fonksiyonunun içine ProductionSystem entegre etmelisin.

        // B) Askerlerin Hareketi
        for (auto& entity : localPlayer.getEntities()) {
            // Unit mi?
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                // Hedefi varsa hareket ettir
                // MovementSystem::move(*unit, unit->getTargetPosition(), dt);
            }
        }

        // C) HUD Güncellemesi (UI)
        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2]); // Wood, Food, Gold, Stone
    }
}

void Game::render() {
    window.clear();

    // Kamerayý aktif et
    window.setView(camera);

    // 1. Oyun Dünyasýný Çiz
    if (stateManager.getState() == GameState::Playing) {
        mapManager.draw(window); // Harita ve Binalar
        localPlayer.renderEntities(window); // Askerler
    }

    // 2. UI Çiz (Kameradan baðýmsýz olmasý için default view'a geç)
    window.setView(window.getDefaultView());
    hud.draw(window);
    uiManager.draw(window);

    window.display();
}

void Game::handleInput(float dt) {
    // Kamera Hareketi (WASD veya Ok Tuþlarý)
    float speed = 500.0f * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camera.move(-speed, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camera.move(speed, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camera.move(0, -speed);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camera.move(0, speed);
}