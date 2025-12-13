#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include "Entity System/Entity Type/Unit.h"
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

    // TEST ASKERÝ OLUÞTUR
    std::shared_ptr<Soldier> testSoldier = std::make_shared<Soldier>();

    // 1. Pozisyon: Haritanýn biraz içinde olsun (100, 100)
    testSoldier->setGridPosition(10, 5);

    // 2. Türünü Belirle (Hýz ve Can deðerleri yüklensin)
    testSoldier->setType(SoldierTypes::Barbarian);

    // 3. Görsellik: Texture yoksa bile görelim diye KIRMIZI yapýyoruz
    testSoldier->getModel().setColor(sf::Color::Red);

    // 4. Oyuncuya ver
    localPlayer.addEntity(testSoldier);

    std::cout << "[SISTEM] Test askeri (Kirmizi) olusturuldu.\n";
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
        uiManager.handleEvent(event); // UI olaylarý (Buton vs.)

        if (event.type == sf::Event::Closed)
            window.close();

        // Sadece oyun oynanýyorken (Menüde deðilken)
        if (stateManager.getState() == GameState::Playing) {

            // Mouse'un DÜNYA üzerindeki konumu (Kamera hesaba katýlarak)
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // --- SOL TIK: SEÇÝM (SELECT) ---
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // UI'ya týklamadýysak (Basit kontrol: Mouse üst barýn altýnda mý?)
                if (pixelPos.y > 40) {
                    localPlayer.selectUnit(window);
                }
            }

            // --- SAÐ TIK: HAREKET (MOVE) ---
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {

                // Seçili askerler varsa onlara emir ver
                if (!localPlayer.selected_entities.empty()) {
                    for (auto& entity : localPlayer.selected_entities) {

                        // Bu entity bir "Unit" mi? (Binalar yürüyemez)
                        // dynamic_pointer_cast ile kontrol ediyoruz
                        if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                            // Askerin hedefini ayarla
                            unit->path.clear(); // Eski yolu unut
                            unit->path.push_back(worldPos); // Yeni hedefi ekle

                            std::cout << "Unit " << unit->entityID << " hedefe gidiyor -> " << worldPos.x << ", " << worldPos.y << "\n";
                        }
                    }
                }
            }
        }
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
        if (stateManager.getState() == GameState::Playing) {

            // --- KAMERA HAREKETÝ ---
            handleInput(dt);

            // --- ASKERLERÝ YÜRÜT ---
            for (auto& entity : localPlayer.getEntities()) {

                // Sadece UNIT olanlar (Askerler/Köylüler) yürür
                if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                    // Gidecek bir yolu var mý?
                    if (!unit->path.empty()) {
                        sf::Vector2f target = unit->path[0]; // Hedef nokta

                        // --- BASÝT HAREKET MATEMATÝÐÝ ---
                        sf::Vector2f currentPos = unit->getPosition();
                        sf::Vector2f direction = target - currentPos;

                        // Pisagor ile mesafe bul
                        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

                        // Hedefe çok yaklaþtýysa dur (Titremeyi önler)
                        if (length < 5.0f) {
                            unit->path.clear(); // Hedefe vardýk, yolu sil
                        }
                        else {
                            // Birim vektör (Yön) hesabý
                            sf::Vector2f normalizedDir = direction / length;

                            // Hýz * Zaman * Yön
                            float speed = unit->getSpeed(); // GameRules'dan gelen hýz
                            sf::Vector2f moveAmount = normalizedDir * speed * dt;

                            unit->move(moveAmount);
                        }
                    }
                }
            }

            // C) HUD Güncellemesi (UI)
            std::vector<int> res = localPlayer.getResources();
            hud.resourceBar.updateResources(res[0], res[3], res[1], res[2]); // Wood, Food, Gold, Stone
        }
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