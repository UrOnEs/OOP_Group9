#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include "Systems/ResourceSystem.h" // - Kaynak sistemi entegrasyonu

#include "Entity System/Entity Type/Unit.h"
#include "Entity System/Entity Type/Villager.h" // - Köylü sýnýfý
#include "Entity System/Entity Type/ResourceGenerator.h" // - Aðaç/Maden sýnýfý
#include "Entity System/Entity Type/Barracks.h" // Kýþla
#include "Entity System/Entity Type/TownCenter.h" // Ana Merkez

#include <algorithm> // std::max ve std::min için
#include <iostream>
#include <vector>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> 

#include "UI/AssetManager.h"
#include "Game/GameRules.h" // - Oyun kurallarý (TileSize, MapWidth vs.)

Game::Game()
    : mapManager(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize)
{
    // --- 1. TAM EKRAN MODU ---
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Empires of Ages - RTS", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // Kamerayý (View) ekran çözünürlüðüne göre ayarla
    camera.setSize(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height));
    camera.setCenter(desktopMode.width / 2.0f, desktopMode.height / 2.0f);
    hud.init(desktopMode.width, desktopMode.height);
    
    initUI();
    initNetwork();
    stateManager.setState(GameState::Playing);

    // Haritayý oluþtur (Aðaçlar vs. burada oluþuyor)
    mapManager.initialize();

    // TEST ASKERÝ OLUÞTUR
    std::shared_ptr<Soldier> testSoldier = std::make_shared<Soldier>();
    testSoldier->setPosition(sf::Vector2f(500.f, 500.f));
    testSoldier->setType(SoldierTypes::Barbarian);
    localPlayer.addEntity(testSoldier);

    std::cout << "[SISTEM] Test askeri olusturuldu.\n";

    // --- 2. 5 ADET KÖYLÜ OLUÞTURMA ---
    for (int i = 0; i < 5; i++) {
        std::shared_ptr<Villager> newVillager = std::make_shared<Villager>();

        // Konumlarýný ayarla (Yan yana dizilsinler: 300,300'den baþlayarak)
        float startX = 300.0f + (i * 50.0f);
        float startY = 300.0f;

        newVillager->setPosition(sf::Vector2f(startX, startY));
        localPlayer.addEntity(newVillager);
    }
    std::cout << "[SISTEM] 5 Koylu oyuna eklendi.\n";

    // ÝNÞAAT GÖRSELLERÝ
    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));
    ghostGridRect.setSize(sf::Vector2f(GameRules::TileSize, GameRules::TileSize));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    // SEÇÝM KUTUSU
    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);
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
    // UI Yüklemeleri (Geçici görsel yoksa konsola yazar)
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

            // KLAVYE KISAYOLLARI
            if (event.type == sf::Event::KeyPressed) {
                //------------------ Tuþlar ------------------------------

                if (event.key.code == sf::Keyboard::H) {
                    enterBuildMode(BuildTypes::House, "assets/buildings/house.png");
                }

                if (event.key.code == sf::Keyboard::B) {
                    // Texture adýný senin klasörüne göre ayarla
                    enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png");
                }

                if (event.key.code == sf::Keyboard::C) {
                    // Texture adýný senin klasörüne göre ayarla
                    enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png");
                }

                if (event.key.code == sf::Keyboard::M) {
                    // Texture adýný senin klasörüne göre ayarla
                    enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png");
                }

                // ESC TUÞU
                if (event.key.code == sf::Keyboard::Escape) {
                    if (isInBuildMode) {
                        cancelBuildMode();
                    }
                    else {
                        window.close();
                    }
                }

                // --- 'T' TUÞU ÝLE ASKER ÜRET ---
                if (event.key.code == sf::Keyboard::T) {
                    // Seçili birim var mý?
                    if (!localPlayer.selected_entities.empty()) {
                        // Ýlk seçilen þey bir Kýþla mý?
                        if (auto barracks = std::dynamic_pointer_cast<Barracks>(localPlayer.selected_entities[0])) {

                            // Barbar üretimini baþlat (Parayý ProductionSystem kontrol edecek)
                            // Þimdilik sadece Barbarian üretiyoruz, ileride okçu vs. için baþka tuþlar eklersin.
                            if (ProductionSystem::startProduction(localPlayer, *barracks, SoldierTypes::Barbarian)) {
                                // Ses çalabilirsin: "Emredersiniz!"
                                // SoundManager::playSound("train_unit"); 
                            }
                        }
                    }
                }
            }

            // Mouse Pozisyonlarý
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            bool isMouseOnUI = hud.isMouseOverUI(pixelPos);

            int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

            if (event.type == sf::Event::MouseButtonPressed) {

                // --- SOL TIK ---
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (isMouseOnUI) continue;

                    if (isInBuildMode) {
                        // ÝNÞAAT MODU
                        GameRules::Cost cost = GameRules::getBuildingCost(pendingBuildingType);
                        if (localPlayer.getResources()[0] >= cost.wood && localPlayer.getResources()[1] >= cost.gold && localPlayer.getResources()[2] >= cost.stone && localPlayer.getResources()[3] >= cost.food) {
                            std::shared_ptr<Building> placedBuilding = mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType);

                            // Eðer bina baþarýyla oluþturulduysa (boþ deðilse)
                            if (placedBuilding != nullptr) {

                                //Binayý oyuncunun listesine ekle
                                localPlayer.addEntity(placedBuilding);

                                // Kaynak düþme iþlemleri
                                localPlayer.addWood(-cost.wood);
                                localPlayer.addGold(-cost.gold);
                                localPlayer.addStone(-cost.stone);
                                localPlayer.addFood(-cost.food);

                                std::cout << "[GAME] Bina insa edildi ve oyuncuya eklendi!\n";

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
                        // SEÇÝM MODU
                        isSelecting = true;
                        selectionStartPos = worldPos;
                        selectionBox.setPosition(selectionStartPos);
                        selectionBox.setSize(sf::Vector2f(0, 0));
                    }
                }

                // --- SAÐ TIK (HAREKET ve ETKÝLEÞÝM) ---
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    if (isInBuildMode) {
                        cancelBuildMode();
                    }
                    else {
                        // 1. Týklanan binayý al (Artýk shared_ptr dönüyor)
                        std::shared_ptr<Building> clickedBuilding = mapManager.getBuildingAt(gridX, gridY);

                        // Dynamic cast, shared_ptr ile þöyle yapýlýr:
                        std::shared_ptr<ResourceGenerator> resGen = std::dynamic_pointer_cast<ResourceGenerator>(clickedBuilding);

                        if (resGen) {
                            // --- EVET, AÐAÇ VAR ---
                            bool sentVillager = false;
                            for (auto& entity : localPlayer.selected_entities) {
                                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {

                                    // ARTIK shared_ptr GÖNDERÝYORUZ
                                    villager->startHarvesting(resGen);

                                    sentVillager = true;
                                }
                            }
                            if (sentVillager) std::cout << "[GAME] Koylu agaca gonderildi\n";
                        }
                        else {
                            // --- HAYIR, BOÞ ZEMÝN: NORMAL YÜRÜ ---

                            // Önce seçili köylülerin hasat iþlemini durdur (Baþka yere týkladýk çünkü)
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

                                // Diðer birimleri engel olarak ekle
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

                    // =========================================================
                    //   PANEL GÜNCELLEME KISMI
                    // =========================================================
                    if (!localPlayer.selected_entities.empty()) {
                        hud.selectedPanel.setVisible(true);

                        // Ýlk seçili birimi al
                        auto entity = localPlayer.selected_entities[0];

                        // 1. Entity'nin yetenek listesini kopyala
                        std::vector<Ability> uiAbilities = entity->getAbilities();

                        // 2. Yeteneklere iþlev (Callback) ata
                        for (auto& ab : uiAbilities) {

                            // --- ÝNÞAAT YETENEKLERÝ (Villager için) ---
                            if (ab.getId() == 1) { // Ev
                                ab.setOnClick([this]() {
                                    this->enterBuildMode(BuildTypes::House, "assets/buildings/house.png");
                                    });
                            }
                            else if (ab.getId() == 2) { // Kýþla
                                ab.setOnClick([this]() {
                                    this->enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png");
                                    });
                            }
                            else if (ab.getId() == 3) { // Çiftlik
                                ab.setOnClick([this]() {
                                    this->enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png"); // Texture adýna dikkat
                                    });
                            }
                            else if(ab.getId() == 4) { //castle
                                ab.setOnClick([this]() {
                                    this->enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png"); // Texture adýna dikkat
                                    });
                            }

                            // --- ÜRETÝM YETENEKLERÝ (Binalar için) ---
                            else if (ab.getId() == 10) { // Kýþla: Asker Üret
                                // Entity'i Barracks tipine dönüþtür
                                if (auto b = std::dynamic_pointer_cast<Barracks>(entity)) {
                                    ab.setOnClick([this, b]() { // b pointer'ýný kopyala
                                        ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Barbarian);
                                        });
                                }
                            }
                        }

                        // 3. Güncellenmiþ listeyi panele gönder
                        hud.selectedPanel.updateSelection(
                            entity->getName(),
                            (int)entity->health,
                            entity->getMaxHealth(),
                            entity->getIcon(),
                            uiAbilities
                        );
                    }
                    else {
                        // Kimse seçili deðilse paneli gizle
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

        // Haritadaki binalarýn listesini al (Referans olarak)
        const auto& allBuildings = mapManager.getBuildings();

        // 1. Entity Güncellemeleri
        for (auto& entity : localPlayer.getEntities()) {
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                unit->update(dt, levelData, mapW, mapH);
            }

            // --- DEÐÝÞÝKLÝK BURADA ---
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                // Artýk parametre olarak binalarý gönderiyoruz
                villager->updateHarvesting(allBuildings);
            }

            if (auto barracks = std::dynamic_pointer_cast<Barracks>(entity)) {
                ProductionSystem::update(localPlayer, *barracks, dt);
            }
        }

        // 2. Kaynak Sistemi
        for (auto& building : mapManager.getBuildings()) {
            if (building) {
                if (auto resGen = std::dynamic_pointer_cast<ResourceGenerator>(building)) {
                    ResourceSystem::update(localPlayer, *resGen, dt);
                }
            }
        }

        // 3. Ölüleri Temizle
        mapManager.removeDeadBuildings();
        // EntityManager::removeDeadEntities eklemen gerekebilir

        // UI Güncelle
        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2],localPlayer);
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

            bool isValid = (gx >= 0 && gx < mapManager.getWidth() &&
                gy >= 0 && gy < mapManager.getHeight());

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

    // Klavye
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  movement.x -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) movement.x += speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    movement.y -= speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  movement.y += speed;

    // Mouse (Edge Panning)
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2u windowSize = window.getSize();

    bool isMouseInside = (mousePos.x >= 0 && mousePos.y >= 0 &&
        mousePos.x < static_cast<int>(windowSize.x) &&
        mousePos.y < static_cast<int>(windowSize.y));

    if (isMouseInside) {
        if (mousePos.x < edgeThreshold) movement.x -= speed;
        else if (mousePos.x > windowSize.x - edgeThreshold) movement.x += speed;

        if (mousePos.y < edgeThreshold) movement.y -= speed;
        else if (mousePos.y > windowSize.y - edgeThreshold) movement.y += speed;
    }

    camera.move(movement);

    // --- KAMERA SINIRLANDIRMA (CLAMPING) ---
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

    float widthInTiles = 2.0f;
    float heightInTiles = 2.0f;

    if (type == BuildTypes::House) {
        widthInTiles = 1.0f;
        heightInTiles = 1.0f;
    }

    float targetWidth = widthInTiles * mapManager.getTileSize();
    float targetHeight = heightInTiles * mapManager.getTileSize();

    sf::Vector2u texSize = tex.getSize();
    ghostBuildingSprite.setScale(targetWidth / texSize.x, targetHeight / texSize.y);
    ghostGridRect.setSize(sf::Vector2f(targetWidth, targetHeight));

    std::cout << "[GAME] Insaat modu aktif.\n";
}

void Game::cancelBuildMode() {
    isInBuildMode = false;
    std::cout << "[GAME] Insaat modu iptal.\n";
}