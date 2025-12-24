#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include "Systems/ResourceSystem.h"

#include "Entity System/Entity Type/Unit.h"
#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/ResourceGenerator.h"
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> 

#include "UI/AssetManager.h"
#include "Game/GameRules.h"

Game::Game()
    : mapManager(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize)
{
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Empires of Ages - RTS", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    camera.setSize(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height));
    camera.setCenter(desktopMode.width / 2.0f, desktopMode.height / 2.0f);
    hud.init(desktopMode.width, desktopMode.height);

    initUI();
    initNetwork();
    stateManager.setState(GameState::Playing);

    mapManager.initialize();

    int startX = 6;
    int startY = 5;

    std::shared_ptr<Building> startTC = mapManager.tryPlaceBuilding(startX, startY, BuildTypes::TownCenter);

    if (startTC) {
        localPlayer.addEntity(startTC);
        std::shared_ptr<Villager> startVil = std::make_shared<Villager>();

        sf::Vector2f spawnPos = startTC->getPosition();
        spawnPos.y += 150.0f;

        startVil->setPosition(spawnPos);
        localPlayer.addEntity(startVil);

        std::cout << "[SISTEM] Oyun baslatildi (1 Castle, 1 Villager).\n";
    }
    else {
        std::cerr << "[HATA] Baslangic binasi yerlestirilemedi! (Alan dolu olabilir)\n";
    }

    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));
    ghostGridRect.setSize(sf::Vector2f(GameRules::TileSize, GameRules::TileSize));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);

    if (bgMusic.openFromFile("assets/sounds/background_music.ogg")) {
        bgMusic.setLoop(true);
        bgMusic.setVolume(30.f);
        bgMusic.play();
    }
}

void Game::initNetwork() {
    networkManager.setLogger([](const std::string& msg) {
        std::cout << "[NETWORK]: " << msg << std::endl;
        });

    lobbyManager = std::make_unique<LobbyManager>(&networkManager, false);

    lobbyManager->setOnGameStart([this]() {
        std::cout << "OYUN BASLIYOR!\n";
        stateManager.setState(GameState::Playing);
        });
}

void Game::initUI() {
    static sf::Texture villagerTex;
    static sf::Texture houseIconTex;

    if (!villagerTex.loadFromFile("assets/icons/villager.png")) {}
    if (!houseIconTex.loadFromFile("assets/icons/house_icon.jpg")) {}

    static std::vector<Ability> testAbilities;
    testAbilities.clear();

    hud.selectedPanel.updateSelection("Selection", 0, 0, nullptr, testAbilities);
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

        uiManager.handleEvent(event);
        hud.handleEvent(event);

        if (event.type == sf::Event::Closed)
            window.close();

        if (stateManager.getState() == GameState::Playing) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::H) {
                    enterBuildMode(BuildTypes::House, "assets/buildings/house.png");
                }
                if (event.key.code == sf::Keyboard::B) {
                    enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png");
                }
                if (event.key.code == sf::Keyboard::C) {
                    enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png");
                }
                if (event.key.code == sf::Keyboard::M) {
                    enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png");
                }
                //HEALTHBAR TEST
                if (event.key.code == sf::Keyboard::K) {
                    if (!localPlayer.selected_entities.empty()) {
                        auto entity = localPlayer.selected_entities[0];

                        if (entity->getIsAlive()) {
                            entity->takeDamage(10.0f); // 10 Can azalt
                            std::cout << "[TEST] Hasar verildi! Kalan Can: " << entity->health << std::endl;
                        }
                    }
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    if (isInBuildMode) cancelBuildMode();
                    else window.close();
                }
                if (event.key.code == sf::Keyboard::T) {
                    if (!localPlayer.selected_entities.empty()) {
                        auto firstEntity = localPlayer.selected_entities[0];
                        if (auto barracks = std::dynamic_pointer_cast<Barracks>(firstEntity)) {
                            ProductionSystem::startProduction(localPlayer, *barracks, SoldierTypes::Barbarian);
                        }
                        else if (auto tc = std::dynamic_pointer_cast<TownCenter>(firstEntity)) {
                            ProductionSystem::startVillagerProduction(localPlayer, *tc);
                        }
                    }
                }
            }

            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);
            bool isMouseOnUI = hud.isMouseOverUI(pixelPos);
            int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (isMouseOnUI) continue;

                    if (isInBuildMode) {
                        GameRules::Cost cost = GameRules::getBuildingCost(pendingBuildingType);
                        if (localPlayer.getResources()[0] >= cost.wood && localPlayer.getResources()[1] >= cost.gold && localPlayer.getResources()[2] >= cost.stone && localPlayer.getResources()[3] >= cost.food) {
                            std::shared_ptr<Building> placedBuilding = mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType);

                            if (placedBuilding != nullptr) {
                                localPlayer.addEntity(placedBuilding);
                                if (placedBuilding->buildingType == BuildTypes::House) {
                                    localPlayer.addUnitLimit(5);
                                }
                                localPlayer.addWood(-cost.wood);
                                localPlayer.addGold(-cost.gold);
                                localPlayer.addStone(-cost.stone);
                                localPlayer.addFood(-cost.food);

                                std::cout << "[GAME] Bina insa edildi!\n";
                                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) cancelBuildMode();
                            }
                            else {
                                std::cout << "[GAME] Buraya insa edilemez!\n";
                            }
                        }
                        else {
                            std::cout << "[GAME] Yetersiz Kaynak!\n";
                            cancelBuildMode();
                        }
                    }
                    else {
                        isSelecting = true;
                        selectionStartPos = worldPos;
                        selectionBox.setPosition(selectionStartPos);
                        selectionBox.setSize(sf::Vector2f(0, 0));
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    if (isInBuildMode) {
                        cancelBuildMode();
                    }
                    else {
                        std::shared_ptr<Building> clickedBuilding = mapManager.getBuildingAt(gridX, gridY);
                        std::shared_ptr<ResourceGenerator> resGen = std::dynamic_pointer_cast<ResourceGenerator>(clickedBuilding);

                        if (resGen) {
                            bool sentVillager = false;
                            for (auto& entity : localPlayer.selected_entities) {
                                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                                    villager->startHarvesting(resGen);
                                    sentVillager = true;
                                }
                            }
                            if (sentVillager) std::cout << "[GAME] Koylu kaynaða gonderildi\n";
                        }
                        else {
                            for (auto& entity : localPlayer.selected_entities) {
                                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                                    villager->stopHarvesting();
                                }
                            }
                            if (gridX >= 0 && gridX < mapManager.getWidth() &&
                                gridY >= 0 && gridY < mapManager.getHeight()) {

                                Point baseTarget = { gridX, gridY };
                                std::set<Point> reservedTiles;
                                const auto& levelData = mapManager.getLevelData();

                                for (const auto& entity : localPlayer.getEntities()) {
                                    if (auto u = std::dynamic_pointer_cast<Unit>(entity)) {
                                        if (!u->isSelected) reservedTiles.insert(u->getGridPoint());
                                    }
                                }

                                for (auto& entity : localPlayer.selected_entities) {
                                    if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                                        Point specificGridTarget = PathFinder::findClosestFreeTile(
                                            baseTarget, levelData, mapManager.getWidth(), mapManager.getHeight(), reservedTiles
                                        );
                                        reservedTiles.insert(specificGridTarget);

                                        std::vector<Point> gridPath = PathFinder::findPath(
                                            unit->getGridPoint(), specificGridTarget, levelData, mapManager.getWidth(), mapManager.getHeight()
                                        );

                                        std::vector<sf::Vector2f> worldPath;
                                        for (const auto& p : gridPath) {
                                            float px = p.x * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                                            float py = p.y * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                                            worldPath.push_back(sf::Vector2f(px, py));
                                        }
                                        unit->setPath(worldPath);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (isSelecting) {
                    isSelecting = false;
                    bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

                    if (std::abs(selectionBox.getSize().x) < 5.0f && std::abs(selectionBox.getSize().y) < 5.0f) {
                        localPlayer.selectUnit(window, camera, shift);
                    }
                    else {
                        localPlayer.selectUnitsInRect(selectionBox.getGlobalBounds(), shift);
                    }
                    selectionBox.setSize(sf::Vector2f(0, 0));

                    if (!localPlayer.selected_entities.empty()) {
                        hud.selectedPanel.setVisible(true);
                        auto entity = localPlayer.selected_entities[0];
                        std::vector<Ability> uiAbilities = entity->getAbilities();

                        for (auto& ab : uiAbilities) {
                            if (ab.getId() == 1) {
                                ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::House, "assets/buildings/house.png"); });
                            }
                            else if (ab.getId() == 2) {
                                ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png"); });
                            }
                            else if (ab.getId() == 3) {
                                ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png"); });
                            }
                            else if (ab.getId() == 4) {
                                ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png"); });
                            }
                            else if (ab.getId() == 10) {
                                if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity)) {
                                    ab.setOnClick([this, tc]() {
                                        ProductionSystem::startVillagerProduction(localPlayer, *tc);
                                        });
                                }
                            }
                            else if (ab.getId() == 11) {
                                if (auto b = std::dynamic_pointer_cast<Barracks>(entity)) {
                                    ab.setOnClick([this, b]() { ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Barbarian); });
                                }
                            }
                            else if (ab.getId() == 12) {
                                if (auto b = std::dynamic_pointer_cast<Barracks>(entity)) {
                                    ab.setOnClick([this, b]() { ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Archer); });
                                }
                            }
                            else if (ab.getId() == 13) {
                                if (auto b = std::dynamic_pointer_cast<Barracks>(entity)) {
                                    ab.setOnClick([this, b]() { ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Wizard); });
                                }
                            }
                            
                        }

                        hud.selectedPanel.updateSelection(
                            entity->getName(), (int)entity->health, entity->getMaxHealth(), entity->getIcon(), uiAbilities
                        );
                    }
                    else {
                        hud.selectedPanel.setVisible(false);
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved) {
                if (isSelecting) {
                    sf::Vector2i currentPixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f currentWorldPos = window.mapPixelToCoords(currentPixelPos, camera);
                    selectionBox.setSize(currentWorldPos - selectionStartPos);
                }
            }
        }
    }
}

void Game::update(float dt) {
    networkManager.update(dt);
    if (stateManager.getState() == GameState::Playing) {
        handleInput(dt);
        const auto& levelData = mapManager.getLevelData();
        int mapW = mapManager.getWidth();
        int mapH = mapManager.getHeight();
        const auto& allBuildings = mapManager.getBuildings();

        for (auto& entity : localPlayer.getEntities()) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                unit->update(dt, levelData, mapW, mapH);
            }
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                villager->updateVillager(dt, allBuildings, localPlayer);
            }
            if (auto barracks = std::dynamic_pointer_cast<Barracks>(entity)) {
                ProductionSystem::update(localPlayer, *barracks, dt);
            }
            if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity)) {
                ProductionSystem::updateTC(localPlayer, *tc, dt);
            }
        }
        for (auto& building : mapManager.getBuildings()) {
            if (building) {
                if (auto resGen = std::dynamic_pointer_cast<ResourceGenerator>(building)) {
                    if (resGen->isWorking()) {
                        ResourceSystem::update(localPlayer, *resGen, dt);
                    }
                }
            }
        }

        //ENTÝTY HEALTH BAR
        if (!localPlayer.selected_entities.empty()) {
            auto entity = localPlayer.selected_entities[0];

            if (entity->getIsAlive()) {
                // Panel görünür olsun
                hud.selectedPanel.setVisible(true);

                // Sadece CAN deðerini güncelle (Butonlarý tekrar oluþturmaz)
                hud.selectedPanel.updateHealth((int)entity->health, entity->getMaxHealth());

            }
            else {
                // Seçili nesne öldüyse seçimi kaldýr ve paneli gizle
                hud.selectedPanel.setVisible(false);
                localPlayer.selected_entities.clear();
            }
        }
        else {
            // Seçim yoksa paneli gizle
            hud.selectedPanel.setVisible(false);
        }

        mapManager.removeDeadBuildings();
        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2], localPlayer);
    }
}

void Game::render() {
    window.clear();
    window.setView(camera);

    if (stateManager.getState() == GameState::Playing) {
        mapManager.draw(window);
        localPlayer.renderEntities(window);

        if (isSelecting) {
            window.draw(selectionBox);
        }

        if (isInBuildMode) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            int gx = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gy = static_cast<int>(worldPos.y / mapManager.getTileSize());

            float snapX = gx * mapManager.getTileSize();
            float snapY = gy * mapManager.getTileSize();

            ghostBuildingSprite.setPosition(snapX, snapY);
            ghostGridRect.setPosition(snapX, snapY);

            // --- YENÝ EKLENEN KISIM: Boyut Kontrolü ---
            // Geniþlik ve yükseklik artýk type'a göre deðiþiyor (enterBuildMode'da ayarlanýyor)
            // Ancak burada mapManager->tryPlaceBuilding ile ayný mantýðý kurmalýyýz.
            // (Basitlik için ghostGridRect boyutunu kullanýyoruz)
            int widthInTiles = (int)(ghostGridRect.getSize().x / mapManager.getTileSize());
            int heightInTiles = (int)(ghostGridRect.getSize().y / mapManager.getTileSize());

            bool isValid = true;
            for (int x = 0; x < widthInTiles; ++x) {
                for (int y = 0; y < heightInTiles; ++y) {
                    if (gx + x < 0 || gx + x >= mapManager.getWidth() ||
                        gy + y < 0 || gy + y >= mapManager.getHeight()) {
                        isValid = false; break;
                    }
                    int idx = (gx + x) + (gy + y) * mapManager.getWidth();
                    if (mapManager.getLevelData()[idx] != 0) {
                        isValid = false; break;
                    }
                }
                if (!isValid) break;
            }

            if (isValid) {
                ghostBuildingSprite.setColor(sf::Color(0, 255, 0, 150));
                ghostGridRect.setOutlineColor(sf::Color::Green);
            }
            else {
                ghostBuildingSprite.setColor(sf::Color(255, 0, 0, 150));
                ghostGridRect.setOutlineColor(sf::Color::Red);
            }

            window.draw(ghostBuildingSprite);
            window.draw(ghostGridRect);
        }
    }

    window.setView(window.getDefaultView());
    hud.draw(window);
    uiManager.draw(window);

    window.display();
}

void Game::handleInput(float dt) {
    if (!window.hasFocus()) return;

    float speed = 1000.0f * dt;
    float edgeThreshold = 30.0f;
    sf::Vector2f movement(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  movement.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    movement.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  movement.y += speed;

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2u windowSize = window.getSize();

    if (mousePos.x >= 0 && mousePos.y >= 0 &&
        mousePos.x < static_cast<int>(windowSize.x) &&
        mousePos.y < static_cast<int>(windowSize.y)) {
        if (mousePos.x < edgeThreshold) movement.x -= speed;
        else if (mousePos.x > windowSize.x - edgeThreshold) movement.x += speed;
        if (mousePos.y < edgeThreshold) movement.y -= speed;
        else if (mousePos.y > windowSize.y - edgeThreshold) movement.y += speed;
    }

    camera.move(movement);

    float mapWidthPixels = static_cast<float>(GameRules::MapWidth * GameRules::TileSize);
    float mapHeightPixels = static_cast<float>(GameRules::MapHeight * GameRules::TileSize);
    sf::Vector2f viewSize = camera.getSize();
    sf::Vector2f viewCenter = camera.getCenter();

    float minX = viewSize.x / 2.0f;
    float minY = viewSize.y / 2.0f;
    float maxX = mapWidthPixels - viewSize.x / 2.0f;
    float maxY = mapHeightPixels - viewSize.y / 2.0f;

    if (maxX < minX) maxX = minX;
    if (maxY < minY) maxY = minY;

    if (viewCenter.x < minX) viewCenter.x = minX;
    if (viewCenter.x > maxX) viewCenter.x = maxX;
    if (viewCenter.y < minY) viewCenter.y = minY;
    if (viewCenter.y > maxY) viewCenter.y = maxY;

    camera.setCenter(viewCenter);
}

void Game::enterBuildMode(BuildTypes type, const std::string& textureName) {
    isInBuildMode = true;
    pendingBuildingType = type;

    sf::Texture& tex = AssetManager::getTexture(textureName);
    ghostBuildingSprite.setTexture(tex);

    // --- DEÐÝÞEN KISIM: BOYUTLARI ÝKÝ KATINA ÇIKAR ---
    float widthInTiles = 4.0f;   // Varsayýlan (Kýþla, Çiftlik vb.)
    float heightInTiles = 4.0f;

    if (type == BuildTypes::House) {
        widthInTiles = 2.0f; // Ev
        heightInTiles = 2.0f;
    }
    else if (type == BuildTypes::TownCenter) {
        widthInTiles = 6.0f; // Ana Bina
        heightInTiles = 6.0f;
    }
    // Diðerleri (Farm, Barrack) 4x4 kalýr

    float targetWidth = widthInTiles * mapManager.getTileSize();
    float targetHeight = heightInTiles * mapManager.getTileSize();

    sf::Vector2u texSize = tex.getSize();
    ghostBuildingSprite.setScale(targetWidth / texSize.x, targetHeight / texSize.y);
    ghostGridRect.setSize(sf::Vector2f(targetWidth, targetHeight));

    std::cout << "[GAME] Insaat modu aktif (Boyut: " << widthInTiles << "x" << heightInTiles << ")\n";
}

void Game::cancelBuildMode() {
    isInBuildMode = false;
    std::cout << "[GAME] Insaat modu iptal.\n";
}