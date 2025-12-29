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

    gameDuration = 0.0f;

    //==================== BAÞLANGIÇ KAYNAKLARI ===================================
    int startX = 6;
    int startY = 5;

    mapManager.clearArea(startX - 2, startY - 2, 12, 12);

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

    // --- DÜÞMAN TEST BÝNASI ---
    enemyPlayer.setTeamColor(TeamColors::Red);

    int enemyX = 15;
    int enemyY = 5;

    mapManager.clearArea(enemyX - 2, enemyY - 2, 12, 12);

    // Haritaya koy
    std::shared_ptr<Building> enemyBarracks = mapManager.tryPlaceBuilding(enemyX, enemyY, BuildTypes::Barrack);

    if (enemyBarracks) {
        // 1. DÜZELTME: Takým rengini ata! (Yoksa AI bunu görmez)
        enemyBarracks->setTeam(TeamColors::Red);

        // Düþman entity listesine ekle (Render ve logic için)
        enemyPlayer.addEntity(enemyBarracks);

        // MapManager zaten onu doðru yere (Grid 15,5'e) koydu. 
        // Eðer setPosition ile oynarsan binanýn görüntüsü kayar ama duvarlarý eski yerinde kalýr.

        std::cout << "[SISTEM] Dusman kislasi (Kirmizi) haritaya eklendi.\n";
    }
    else {
        std::cerr << "[HATA] Enemy binasi yerlestirilemedi! (Alan dolu olabilir)\n";
    }

    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));
    ghostGridRect.setSize(sf::Vector2f(GameRules::TileSize, GameRules::TileSize));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);

    //----------------------------- ArkaPlan Müziði ----------------------------------------
    if (bgMusic.openFromFile("assets/sounds/background_music.ogg")) {
        bgMusic.setLoop(true);
        bgMusic.setVolume(GameRules::BackgroundMusicVolume);
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

// ======================================================================================
//                                  ANA EVENT DÖNGÜSÜ
// ======================================================================================

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {

        uiManager.handleEvent(event);
        hud.handleEvent(event);

        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (stateManager.getState() == GameState::Playing) {

            // 1. KLAVYE GÝRDÝLERÝ
            if (event.type == sf::Event::KeyPressed) {
                handleKeyboardInput(event);
            }

            // 2. MOUSE GÝRDÝLERÝ (Týklamalar)
            if (event.type == sf::Event::MouseButtonPressed ||
                event.type == sf::Event::MouseButtonReleased ||
                event.type == sf::Event::MouseMoved) {
                handleMouseInput(event);
            }
        }
    }
}

// ======================================================================================
//                                  KLAVYE KONTROLLERÝ
// ======================================================================================

void Game::handleKeyboardInput(const sf::Event& event) {
    // Kýsayollar
    if (event.key.code == sf::Keyboard::H) enterBuildMode(BuildTypes::House, "assets/buildings/house.png");
    if (event.key.code == sf::Keyboard::B) enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png");
    if (event.key.code == sf::Keyboard::C) enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png");
    if (event.key.code == sf::Keyboard::M) enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png");

    // Ýptal / Çýkýþ
    if (event.key.code == sf::Keyboard::Escape) {
        if (isInBuildMode) cancelBuildMode();
        else window.close();
    }

    // T: TRAIN (Üretim)
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

    // D: DESTROY (Yýkma)
    if (event.key.code == sf::Keyboard::D) {
        if (!localPlayer.selected_entities.empty()) {
            bool destroyed = false;
            for (auto& entity : localPlayer.selected_entities) {
                if (auto building = std::dynamic_pointer_cast<Building>(entity)) {
                    building->isAlive = false;
                    destroyed = true;
                }
            }
            if (destroyed) {
                localPlayer.selected_entities.clear();
                hud.selectedPanel.setVisible(false);
                std::cout << "[GAME] Bina yikildi.\n";
            }
        }
    }

    // K: Hasar Testi (Debug)
    if (event.key.code == sf::Keyboard::K) {
        if (!localPlayer.selected_entities.empty()) {
            localPlayer.selected_entities[0]->takeDamage(10.0f);
        }
    }
}

// ======================================================================================
//                                  MOUSE KONTROLLERÝ
// ======================================================================================

void Game::handleMouseInput(const sf::Event& event) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

    // UI üzerindeyse oyun dünyasýna týklamayý engelle
    if (event.type == sf::Event::MouseButtonPressed && hud.isMouseOverUI(pixelPos)) return;

    // --- SOL TIK ---
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        onLeftClick(worldPos, pixelPos);
    }

    // --- SAÐ TIK ---
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        onRightClick(worldPos);
    }

    // --- SEÇÝM KUTUSU GÜNCELLEME (Mouse Sürükleme) ---
    else if (event.type == sf::Event::MouseMoved && isSelecting) {
        selectionBox.setSize(worldPos - selectionStartPos);
    }

    // --- SEÇÝM BÝTÝRME (Mouse Býrakma) ---
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isSelecting) {
            isSelecting = false;
            bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

            // Nokta mý yoksa Kutu mu?
            if (std::abs(selectionBox.getSize().x) < 5.0f && std::abs(selectionBox.getSize().y) < 5.0f) {
                localPlayer.selectUnit(window, camera, shift);
            }
            else {
                localPlayer.selectUnitsInRect(selectionBox.getGlobalBounds(), shift);
            }
            selectionBox.setSize(sf::Vector2f(0, 0));

            // HUD Güncellemesi
            if (!localPlayer.selected_entities.empty()) {
                hud.selectedPanel.setVisible(true);
                auto entity = localPlayer.selected_entities[0];

                // Buton Callback'lerini Ayarla (Lambda ile)
                std::vector<Ability> uiAbilities = entity->getAbilities();
                for (auto& ab : uiAbilities) {
                    // Buradaki ID'leri Ability eklerken verdiðin ID'lere göre ayarla
                    if (ab.getId() == 1) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::House, "assets/buildings/house.png"); });
                    else if (ab.getId() == 2) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png"); });
                    else if (ab.getId() == 3) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png"); });
                    else if (ab.getId() == 4) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png"); });

                    else if (ab.getId() == 10) { // Köylü Üret
                        if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity))
                            ab.setOnClick([this, tc]() { ProductionSystem::startVillagerProduction(localPlayer, *tc); });
                    }
                    else if (ab.getId() == 11) { // Barbar
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b]() { ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Barbarian); });
                    }
                    else if (ab.getId() == 12) { // Okçu
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b]() { ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Archer); });
                    }
                    else if (ab.getId() == 13) { // Wizard
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b]() { ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Wizard); });
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
}

// ======================================================================================
//                                  SOL TIK (ÝNÞAAT / SEÇÝM)
// ======================================================================================

void Game::onLeftClick(const sf::Vector2f& worldPos, const sf::Vector2i& pixelPos) {
    int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
    int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

    if (isInBuildMode) {
        // --- ÝNÞAAT YAPMA (Mevcut kodlarýn ayný) ---
        GameRules::Cost cost = GameRules::getBuildingCost(pendingBuildingType);

        bool canAfford = localPlayer.getResources()[0] >= cost.wood &&
            localPlayer.getResources()[1] >= cost.gold &&
            localPlayer.getResources()[2] >= cost.stone &&
            localPlayer.getResources()[3] >= cost.food;

        if (canAfford) {
            std::shared_ptr<Building> placed = mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType);
            if (placed) {
                localPlayer.addEntity(placed);
                if (placed->buildingType == BuildTypes::House) localPlayer.addUnitLimit(5);

                localPlayer.addWood(-cost.wood);
                localPlayer.addGold(-cost.gold);
                localPlayer.addStone(-cost.stone);
                localPlayer.addFood(-cost.food);

                std::cout << "[GAME] Bina insa edildi!\n";
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) cancelBuildMode();
            }
            else {
                std::cout << "[GAME] Alan dolu veya gecersiz!\n";
            }
        }
        else {
            std::cout << "[GAME] Yetersiz Kaynak!\n";
            cancelBuildMode();
        }
    }
    else {
        bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

        // 1. Önce KENDÝ birimlerimizi seçmeyi dene
        localPlayer.selectUnit(window, camera, shift);

        // 2. Eðer bizden kimse seçilmediyse (veya boþluða týklandýysa), DÜÞMANLARA bak
        if (localPlayer.selected_entities.empty()) {
            enemyPlayer.selectUnit(window, camera, false); // Düþman için shift yok

            if (!enemyPlayer.selected_entities.empty()) {
                // Düþman seçildi!
                auto entity = enemyPlayer.selected_entities[0];
                std::vector<Ability> emptyAbilities; // Düþmanýn yeteneklerini gösterme

                hud.selectedPanel.setVisible(true);
                hud.selectedPanel.updateSelection(
                    "[DUSMAN] " + entity->getName(),
                    (int)entity->health, entity->getMaxHealth(),
                    entity->getIcon(),
                    emptyAbilities
                );
            }
            else {
                hud.selectedPanel.setVisible(false);
            }
        }
        else {
            // Bizim birimimiz seçildiyse düþman seçimini temizle
            enemyPlayer.selected_entities.clear();

            // HUD Güncelle (Bizim birim için)
            auto entity = localPlayer.selected_entities[0];
            hud.selectedPanel.setVisible(true);

            // Butonlarý güncellemek için handleMouseInput içinde yaptýðýn mantýðý 
            // burada da çaðýrabilirsin veya HUD update'e býrakabilirsin.
            // Þimdilik temel bilgileri gösterelim:
            hud.selectedPanel.updateSelection(
                entity->getName(),
                (int)entity->health, entity->getMaxHealth(),
                entity->getIcon(),
                entity->getAbilities()
            );
        }

        // 3. SEÇÝM KUTUSU BAÞLAT (Her zaman çalýþsýn ki sürükleyebilelim)
        isSelecting = true;
        selectionStartPos = worldPos;
        selectionBox.setPosition(selectionStartPos);
        selectionBox.setSize(sf::Vector2f(0, 0));
    }
}

// ======================================================================================
//                                  SAÐ TIK (HAREKET / SALDIRI / HASAT)
// ======================================================================================

void Game::onRightClick(const sf::Vector2f& worldPos) {
    if (isInBuildMode) {
        cancelBuildMode();
        return;
    }

    int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
    int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

    // 1. Týklanan þey bir BÝNA mý?
    std::shared_ptr<Building> clickedBuilding = mapManager.getBuildingAt(gridX, gridY);
    auto resGen = std::dynamic_pointer_cast<ResourceGenerator>(clickedBuilding);

    // 2. Týklanan sey bir UNIT mi? (Düþman kontrolü)
    std::shared_ptr<Entity> clickedEnemyUnit = nullptr;
    for (auto& ent : enemyPlayer.getEntities()) {

        // --- SADECE CANLI OLANLARI HEDEF AL ---
        if (!ent->getIsAlive()) continue;
        // ------------------------------------------------

        // Týklanan noktaya 20 piksel yakýnlýkta bir düþman var mý?
        sf::Vector2f diff = ent->getPosition() - worldPos;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < 20.0f * 20.0f) { // 20px yarýçap
            clickedEnemyUnit = ent;
            break;
        }
    }

    // --- KARAR MEKANÝZMASI ---

    if (clickedBuilding) {
        // --- A. BÝR BÝNAYA TIKLANDI ---

        // 1. Köylüleri Yönet (Kaynak ise hasat et)
        if (resGen) {
            bool sentVillager = false;
            for (auto& entity : localPlayer.selected_entities) {
                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                    villager->startHarvesting(resGen);
                    sentVillager = true;
                }
            }
            if (sentVillager) std::cout << "[GAME] Koyluler hasada gonderildi.\n";
        }

        // 2. Askerleri Yönet (Düþman veya Bina ise Saldýr)
        bool sentSoldier = false;
        for (auto& entity : localPlayer.selected_entities) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->setTarget(clickedBuilding);
                sentSoldier = true;
            }
        }
        if (sentSoldier) std::cout << "[GAME] Binaya saldiri emri!\n";

    }
    else if (clickedEnemyUnit) {
        // --- B. DÜÞMAN ASKERÝNE SALDIRI ---
        std::cout << "[GAME] Dusman askerine saldiri emri!\n";
        for (auto& entity : localPlayer.selected_entities) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->setTarget(clickedEnemyUnit);
            }
        }
    }
    else {
        // --- C. BOÞ YERE TIKLANDI (FORCE MOVE / HAREKET) ---

        // 1. Köylüleri durdur
        for (auto& entity : localPlayer.selected_entities) {
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                villager->stopHarvesting();
            }
        }

        // 2. Asker Hareket Mantýðý
        if (gridX >= 0 && gridX < mapManager.getWidth() &&
            gridY >= 0 && gridY < mapManager.getHeight()) {

            Point baseTarget = { gridX, gridY };
            std::set<Point> reservedTiles;
            const auto& levelData = mapManager.getLevelData();

            // Sadece sabit engelleri ve "seçili olmayan" birimleri rezerve et.
            // Seçili olanlar zaten yürüyeceði için birbirini engellemesin.
            for (const auto& entity : localPlayer.getEntities()) {
                if (entity->getIsAlive() && !entity->isSelected) {
                    reservedTiles.insert(entity->getGridPoint());
                }
            }

            for (auto& entity : localPlayer.selected_entities) {
                if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                    // Askerler için özel durum ayarla
                    if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                        soldier->setForceMove(); // State = Moving yapar
                    }

                    // Hedef bul
                    Point specificGridTarget = PathFinder::findClosestFreeTile(
                        baseTarget, levelData, mapManager.getWidth(), mapManager.getHeight(), reservedTiles
                    );
                    reservedTiles.insert(specificGridTarget); // Burayý kaptýk

                    // Yol Hesapla
                    std::vector<Point> gridPath = PathFinder::findPath(
                        unit->getGridPoint(), specificGridTarget, levelData, mapManager.getWidth(), mapManager.getHeight()
                    );

                    // Eðer yol bulunamadýysa (boþ vektörse) hareket etmeyecektir.
                    // Bu durumda askeri Moving modundan çýkarmamýz lazým yoksa takýlý kalýr.
                    if (gridPath.empty()) {
                        if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                            soldier->state = SoldierState::Idle; // Yol yoksa IDLE kal
                        }
                    }
                    else {
                        // Yol bulundu, çevir ve ata
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
void Game::update(float dt) {
    networkManager.update(dt);
    if (stateManager.getState() == GameState::Playing) {
        handleInput(dt);
        const auto& levelData = mapManager.getLevelData();
        int mapW = mapManager.getWidth();
        int mapH = mapManager.getHeight();
        const auto& allBuildings = mapManager.getBuildings();

        // --- ENTITY GÜNCELLEMELERÝ ---
        for (auto& entity : localPlayer.getEntities()) {

            // 1. Fiziksel Hareket (Hepsi için)
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                unit->update(dt, levelData, mapW, mapH);
            }

            // 2. Köylü Mantýðý
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                villager->updateVillager(dt, allBuildings, localPlayer);
            }

            // 3. ASKER MANTIÐI (BU EKSÝKTÝ - SORUNU ÇÖZEN KISIM)
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                // Senin askerin -> Düþman Player'ýn birimlerine bakar
                soldier->updateSoldier(dt, enemyPlayer.getEntities());
            }
            // -----------------------------------------------------

            // 4. Binalar
            if (auto barracks = std::dynamic_pointer_cast<Barracks>(entity)) {
                ProductionSystem::update(localPlayer, *barracks, dt, mapManager);
            }
            if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity)) {
                ProductionSystem::updateTC(localPlayer, *tc, dt, mapManager);
            }
        }

        // --- KAYNAK SÝSTEMÝ (ÇÝFTLÝKLER) ---
        for (auto& building : mapManager.getBuildings()) {
            if (building) {
                if (auto resGen = std::dynamic_pointer_cast<ResourceGenerator>(building)) {
                    if (resGen->isWorking()) {
                        ResourceSystem::update(localPlayer, *resGen, dt);
                    }
                }
            }
        }

        // --- UI GÜNCELLEME ---
        // Önce kendi seçimimize bak
        if (!localPlayer.selected_entities.empty()) {
            auto entity = localPlayer.selected_entities[0];
            if (entity->getIsAlive()) {
                hud.selectedPanel.setVisible(true);
                hud.selectedPanel.updateHealth((int)entity->health, entity->getMaxHealth());
            }
            else {
                hud.selectedPanel.setVisible(false);
                localPlayer.selected_entities.clear();
            }
        }
        // Eðer bizde seçim yoksa düþmana bak
        else if (!enemyPlayer.selected_entities.empty()) {
            auto entity = enemyPlayer.selected_entities[0];
            if (entity->getIsAlive()) {
                hud.selectedPanel.setVisible(true);
                hud.selectedPanel.updateHealth((int)entity->health, entity->getMaxHealth());
            }
            else {
                hud.selectedPanel.setVisible(false);
                enemyPlayer.selected_entities.clear();
            }
        }
        else {
            hud.selectedPanel.setVisible(false);
        }

        // =================================================================
        //                     DÜÞMAN YAPAY ZEKASI (AI)
        // =================================================================

        // 0. Oyun Süresini Artýr
        gameDuration += dt;

        // 1. Düþman Kýþlasýný Bul ve Üretim Yaptýr
        for (auto& building : mapManager.getBuildings()) {
            if (auto barracks = std::dynamic_pointer_cast<Barracks>(building)) {

                if (barracks->getTeam() == TeamColors::Red) {

                    barracks->updateTimer(dt);

                    // ---------------- SÜRE KONTROLÜ ---------------------------
                    // Oyunun ilk saniyeleri boyunca düþman üretim yapmasýn.
                    // Bu sayede oyuncu geliþmek için zaman kazanýr.
                    if (gameDuration < 30.0f) {
                        continue;
                    }
                    // -----------------------------------

                    // A. Boþsa Üretime Baþla
                    if (!barracks->getIsProducing()) {
                        // Artýk üretim yavaþ yavaþ baþlasýn
                        barracks->startTraining(SoldierTypes::Barbarian, 10.0f);
                    }

                    // B. Üretim Bittiyse Askeri Çýkar
                    ProductionSystem::update(enemyPlayer, *barracks, dt, mapManager);
                }
            }
        }

        // 2. Düþman Askerlerini Yönet
        for (auto& entity : enemyPlayer.getEntities()) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {

                // Fiziksel hareket
                soldier->update(dt, levelData, mapW, mapH);

                // Zihinsel hareket: Düþman askeri -> Senin (LocalPlayer) birimlerine bakar
                soldier->updateSoldier(dt, localPlayer.getEntities());

                // NOT: Artýk buradaki "targetToAttack" arama döngüsüne gerek kalmadý!
                // updateSoldier içinde zaten otomatik arýyor.
                // Eski uzun döngüyü silebilirsin. Kod çok temizlendi.
            }
        }

        mapManager.removeDeadBuildings();

        // --- ÖLÜ ASKER TEMÝZLÝÐÝ ---
        localPlayer.removeDeadEntities();
        enemyPlayer.removeDeadEntities();
        // ----------------------------------

        enemyPlayer.renderEntities(window);
        localPlayer.renderEntities(window); // Render mantýðý render() içinde ama entity güncellemesi burada

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
        enemyPlayer.renderEntities(window); //düþman

        for (auto& entity : localPlayer.getEntities()) {
            if (entity->getIsAlive()) entity->renderEffects(window);
        }
        for (auto& entity : enemyPlayer.getEntities()) {
            if (entity->getIsAlive()) entity->renderEffects(window);
        }

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