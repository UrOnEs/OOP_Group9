#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include "Entity System/Entity Type/Unit.h"
#include <iostream>
#include <vector>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> 

Game::Game()
    : mapManager(50, 50, 32)
{
    window.create(sf::VideoMode(1280, 720), "Empires of Ages - RTS");
    window.setFramerateLimit(60);

    // Kamerayý baþlat
    camera.setSize(1280, 720);
    camera.setCenter(1280 / 2, 720 / 2);

    initUI();      // <-- Test verileri burada yükleniyor
    initNetwork();
    stateManager.setState(GameState::Playing);

    // Haritayý oluþtur
    mapManager.initialize();

    // TEST ASKERÝ OLUÞTUR
    std::shared_ptr<Soldier> testSoldier = std::make_shared<Soldier>();
    testSoldier->setPosition(sf::Vector2f(500.f, 500.f));
    testSoldier->setType(SoldierTypes::Barbarian);
    testSoldier->getModel().setFillColor(sf::Color::Red);
    localPlayer.addEntity(testSoldier);

    std::cout << "[SISTEM] Test askeri (Kirmizi) olusturuldu.\n";
}

void Game::initNetwork() {
    networkManager.setLogger([](const std::string& msg) {
        std::cout << "[NETWORK]: " << msg << std::endl;
        });

    // networkManager.startServer(54000); 
    lobbyManager = std::make_unique<LobbyManager>(&networkManager, false);

    lobbyManager->setOnGameStart([this]() {
        std::cout << "OYUN BASLIYOR!\n";
        stateManager.setState(GameState::Playing);
        });
}

// --- GERÝ GETÝRÝLEN KISIM: TEST VERÝLERÝ (MOCK DATA) ---
void Game::initUI() {
    // Static yaparak hafýzada kalmalarýný saðlýyoruz (Geçici test için)
    static sf::Texture villagerTex;
    static sf::Texture houseIconTex;
    static sf::Texture millIconTex;

    // Resim yüklemeyi dene (Yoksa konsola yazar, çökmez)
    if (!villagerTex.loadFromFile("assets/icons/villager.png")) std::cout << "Villager png yok (Test)\n";
    if (!houseIconTex.loadFromFile("assets/icons/house_icon.jpg")) std::cout << "House png yok (Test)\n";
    if (!millIconTex.loadFromFile("assets/icons/mill_icon.png")) std::cout << "Mill png yok (Test)\n";

    static std::vector<AbilityInfo> testAbilities;
    testAbilities.clear();

    // 1. Yetenek: Ev Yap
    AbilityInfo buildHouse;
    buildHouse.id = 1;
    buildHouse.name = "Build House";
    buildHouse.costText = "Cost: 30 Wood";
    buildHouse.description = "Population space for 5 units.\nNecessary for expanding army.";
    buildHouse.iconTexture = &houseIconTex;
    testAbilities.push_back(buildHouse);

    // 2. Yetenek: Deðirmen Yap
    AbilityInfo buildMill;
    buildMill.id = 2;
    buildMill.name = "Build Mill";
    buildMill.costText = "Cost: 100 Wood";
    buildMill.description = "Drop off point for Food.\nIncreases farm efficiency.";
    buildMill.iconTexture = &millIconTex;
    testAbilities.push_back(buildMill);

    // Panele yükle
    // Not: Týklama testi için callback, SelectedObjectPanel.cpp içinde tanýmlandý.
    hud.selectedPanel.updateSelection("Test Villager", 25, 40, &villagerTex, testAbilities);

    std::cout << "[UI] Test verileri panele yuklendi.\n";
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

        // --- YENÝ EVENT SÝSTEMÝ ENTEGRASYONU ---
        uiManager.handleEvent(event);
        hud.handleEvent(event); // Panel týklamalarý burada iþleniyor

        if (event.type == sf::Event::Closed)
            window.close();

        if (stateManager.getState() == GameState::Playing) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

            // UI Korumasý: Mouse panelin üzerindeyse haritaya týklama
            // (Panel koordinatý 600 varsayýldý, HUD.cpp ile ayný olmalý)
            bool isMouseOnPanel = (pixelPos.y > 600);
            bool isMouseOnTopBar = (pixelPos.y < 50);

            if ((event.type == sf::Event::MouseButtonPressed) && (isMouseOnTopBar || isMouseOnPanel)) {
                continue;
            }

            // Harita Kontrolleri
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // SOL TIK: SEÇÝM
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                localPlayer.selectUnit(window);
            }

            // SAÐ TIK: HAREKET
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                int targetGridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
                int targetGridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

                if (targetGridX >= 0 && targetGridX < mapManager.getWidth() &&
                    targetGridY >= 0 && targetGridY < mapManager.getHeight()) {

                    Point baseTarget = { targetGridX, targetGridY };
                    std::set<Point> reservedTiles;
                    const auto& levelData = mapManager.getLevelData();

                    for (const auto& entity : localPlayer.getEntities()) {
                        if (auto u = std::dynamic_pointer_cast<Unit>(entity)) {
                            if (!u->isSelected) {
                                reservedTiles.insert(u->getGridPoint());
                            }
                        }
                    }

                    for (auto& entity : localPlayer.selected_entities) {
                        if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                            Point specificGridTarget = PathFinder::findClosestFreeTile(
                                baseTarget, levelData, mapManager.getWidth(), mapManager.getHeight(), reservedTiles
                            );
                            reservedTiles.insert(specificGridTarget);

                            float pixelX = specificGridTarget.x * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                            float pixelY = specificGridTarget.y * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                            unit->moveTo(sf::Vector2f(pixelX, pixelY));
                        }
                    }
                }
            }
        }
    }
}

void Game::update(float dt) {
    networkManager.update(dt);

    if (stateManager.getState() == GameState::Playing) {
        handleInput(dt);

        // Artýk burada UI kontrolü yapmýyoruz, processEvents içinde yapýlýyor.

        const auto& levelData = mapManager.getLevelData();
        int mapW = mapManager.getWidth();
        int mapH = mapManager.getHeight();

        for (auto& entity : localPlayer.getEntities()) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                unit->update(dt, levelData, mapW, mapH);
            }
        }

        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2]);
    }
}

void Game::render() {
    window.clear();

    // 1. Oyun Dünyasý
    window.setView(camera);
    if (stateManager.getState() == GameState::Playing) {
        mapManager.draw(window);
        localPlayer.renderEntities(window);
    }

    // 2. UI
    window.setView(window.getDefaultView());
    hud.draw(window);
    uiManager.draw(window);

    window.display();
}

void Game::handleInput(float dt) {
    float speed = 500.0f * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) camera.move(-speed, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) camera.move(speed, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) camera.move(0, -speed);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) camera.move(0, speed);
}