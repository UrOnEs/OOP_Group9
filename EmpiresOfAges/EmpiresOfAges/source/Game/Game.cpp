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

#include <thread>             // std::this_thread::sleep_for için gerekli
#include <chrono>             // std::chrono::milliseconds için gerekli
#include "Network/NetServer.h" // NetServer fonksiyonlarýný (setOnPacket) kullanmak için gerekli
#include "Network/NetClient.h" // NetClient fonksiyonlarýný kullanmak için gerekli

// --- YENÝ EKLENEN INCLUDE ---
#include "Map/FogOfWar.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> 

#include "UI/AssetManager.h"
#include "Game/GameRules.h"

Game::Game(bool isHost, std::string serverIp)
    : mapManager(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize),
    m_isHost(isHost),     // Kaydet
    m_serverIp(serverIp)  // Kaydet
{
    // 1. Pencere ve Görünüm Ayarlarý
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Empires of Ages - RTS", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    camera.setSize(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height));
    camera.setCenter(desktopMode.width / 2.0f, desktopMode.height / 2.0f);

    // 2. Arayüz ve Að Sistemlerini Baþlat
    hud.init(desktopMode.width, desktopMode.height);
    initUI();
    initNetwork(); // Bu fonksiyon ileride startMatch'i tetikleyecek

    // 3. Savaþ Sisi (Harita içeriðinden baðýmsýzdýr, güvenle oluþturulabilir)
    m_fogOfWar = std::make_unique<FogOfWar>(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize);

    // 4. Ýnþaat ve Seçim Görselleri (UI)
    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));

    ghostGridRect.setSize(sf::Vector2f(GameRules::TileSize, GameRules::TileSize));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);

    // 5. Arka Plan Müziði
    if (bgMusic.openFromFile("assets/sounds/background_music.ogg")) {
        bgMusic.setLoop(true);
        bgMusic.setVolume(GameRules::BackgroundMusicVolume);
        bgMusic.play();
    }

    // --- DÝKKAT ---
    // mapManager.initialize(), clearArea(), addEntity() gibi harita ve birim
    // oluþturma kodlarýnýn tamamý buradan SÝLÝNDÝ ve startMatch() fonksiyonuna taþýndý.
    // Bu sayede oyun, sunucudan seed gelmeden haritaya dokunmaz ve çökmez.
}

void Game::startMatch(unsigned int seed) {
    std::cout << "[GAME] Mac baslatiliyor. Seed: " << seed << std::endl;

    // A. Haritayý Seed ile Oluþtur
    mapManager.initialize(seed);

    // B. Önceki Entityleri Temizle (Varsa)
    localPlayer.entities.clear();
    localPlayer.selected_entities.clear();
    enemyPlayer.entities.clear();
    enemyPlayer.selected_entities.clear();
    // ResourceManager sýfýrlama vb. gerekirse buraya eklersin.

    // C. Baþlangýç Binalarýný Yerleþtir (Constructor'dan aldýðýmýz kodlar)
    // ====================================================================
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
    }

    // Düþman Test Binasý
    enemyPlayer.setTeamColor(TeamColors::Red);
    int enemyX = 15;
    int enemyY = 5;
    mapManager.clearArea(enemyX - 2, enemyY - 2, 12, 12);
    std::shared_ptr<Building> enemyBarracks = mapManager.tryPlaceBuilding(enemyX, enemyY, BuildTypes::Barrack);

    if (enemyBarracks) {
        enemyBarracks->setTeam(TeamColors::Red);
        enemyPlayer.addEntity(enemyBarracks);
    }

    // Oyunu oynanýyor moduna al
    stateManager.setState(GameState::Playing);
}

void Game::initNetwork() {
    networkManager.setLogger([](const std::string& msg) {
        // std::cout << "[NETWORK]: " << msg << std::endl;
    });

    if (m_isHost) {
        // --- HOST (SUNUCU) ---
        unsigned short port = 54000;
        if (networkManager.startServer(port)) {
            // 1. LobbyManager Oluþtur
            lobbyManager = std::make_unique<LobbyManager>(&networkManager, true);
            
            // 2. CALLBACK BAÐLANTISI (BU EKSÝKTÝ!)
            // Gelen paketleri LobbyManager'a yönlendiriyoruz
            networkManager.server()->setOnPacket([this](uint64_t id, sf::Packet& pkt) {
                if (lobbyManager) lobbyManager->handleIncomingPacket(id, pkt);
            });

            // 3. Lobby Baþlat
            lobbyManager->start(1, "HostPlayer");
            lobbyManager->toggleReady(true);
        } else {
            std::cerr << "[GAME] HATA: Sunucu baslatilamadi!" << std::endl;
            return;
        }
    }
    else {
        // --- CLIENT (ÝSTEMCÝ) ---
        std::cout << "[GAME] Sunucuya baglaniliyor: " << m_serverIp << "...\n";
        
        if (networkManager.startClient(m_serverIp, 54000)) {
            // 1. LobbyManager Oluþtur
            lobbyManager = std::make_unique<LobbyManager>(&networkManager, false);

            // 2. CALLBACK BAÐLANTISI (BU EKSÝKTÝ!)
            // Gelen paketleri LobbyManager'a yönlendiriyoruz
            networkManager.client()->setOnPacket([this](sf::Packet& pkt) {
                if (lobbyManager) lobbyManager->handleIncomingPacket(0, pkt); // Client için ID önemsiz (0)
            });

            // 3. Lobby Baþlat
            lobbyManager->start(0, "ClientPlayer");
            lobbyManager->toggleReady(true);
            std::cout << "[GAME] Client baglandi. Oyun baslamasi bekleniyor...\n";
        } else {
             std::cerr << "[GAME] HATA: Sunucuya baglanilamadi!" << std::endl;
             return;
        }
    }

    // --- ORTAK Callback ---
    // Oyun baþla sinyali gelince ne yapacaðýz?
    if (lobbyManager) {
        lobbyManager->setOnGameStart([this]() {
            unsigned int seed = lobbyManager->getGameSeed();
            this->startMatch(seed);
        });
    }

    // --- HOST ÝSE OYUNU HEMEN BAÞLAT ---
    if (m_isHost && lobbyManager) {
        // Client'ýn baðlanmasý için minik bir bekleme (UDP olduðu için)
        // Ýdeal dünyada client'tan "Ben geldim" onayý bekleriz ama þimdilik sleep yeterli.
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        
        std::cout << "[GAME] Sunucu baslatildi. Sinyal gonderiliyor...\n";
        lobbyManager->startGame();
    }
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

                //Productiondan gelen output nasýl deðerlendirilecek
                auto handleProductionResult = [this](ProductionResult result) {
                    switch (result) {
                    case ProductionResult::Success:
                        // Baþarýlý (Ses çalýnabilir)
                        break;
                    case ProductionResult::PopulationFull:
                        showWarning("Nufus Limiti Dolu! Ev insa edin.");
                        break;
                    case ProductionResult::InsufficientFood:
                        showWarning("Yetersiz Kaynak: Yemek gerekli.");
                        break;
                    case ProductionResult::InsufficientWood:
                        showWarning("Yetersiz Kaynak: Odun gerekli.");
                        break;
                    case ProductionResult::InsufficientGold:
                        showWarning("Yetersiz Kaynak: Altin gerekli.");
                        break;
                    case ProductionResult::QueueFull:
                        showWarning("Uretim kuyrugu dolu!");
                        break;
                    default:
                        break;
                    }
                    };

                // Buton Callback'lerini Ayarla (Lambda ile)
                std::vector<Ability> uiAbilities = entity->getAbilities();
                for (auto& ab : uiAbilities) {
                    if (ab.getId() == 1) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::House, "assets/buildings/house.png"); });
                    else if (ab.getId() == 2) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png"); });
                    else if (ab.getId() == 3) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png"); });
                    else if (ab.getId() == 4) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png"); });

                    else if (ab.getId() == 10) { // Köylü Üret
                        if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity))
                            ab.setOnClick([this, tc, handleProductionResult]() {
                            // Kontrolü ProductionSystem yapsýn, sonucu alalým
                            ProductionResult res = ProductionSystem::startVillagerProduction(localPlayer, *tc);
                            // Sonucu yardýmcý fonksiyona gönderelim
                            handleProductionResult(res);
                                });
                    }
                    else if (ab.getId() == 11) { // Barbar
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Barbarian);
                            handleProductionResult(res);
                                });
                    }
                    else if (ab.getId() == 12) { // Okçu
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Archer);
                            handleProductionResult(res);
                                });
                    }
                    else if (ab.getId() == 13) { // Wizard
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Wizard);
                            handleProductionResult(res);
                                });
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
        // --- ÝNÞAAT YAPMA ---
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
            showWarning("Yetersiz Kaynak!");
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
                auto entity = enemyPlayer.selected_entities[0];

                // DÜZELTME: Eðer düþman görünmezse (sis içindeyse) seçimi iptal et
                if (m_fogOfWar && !m_fogOfWar->isVisible(entity->getPosition().x, entity->getPosition().y)) {
                    // Bina deðilse ve görünmüyorsa seçme. (Binalar explored alanda görülebilir, bu basit kontrol þimdilik yeterli)
                    if (!std::dynamic_pointer_cast<Building>(entity)) {
                        enemyPlayer.selected_entities.clear();
                        hud.selectedPanel.setVisible(false);
                        return; // Fonksiyondan çýk
                    }
                }

                // Düþman seçildi ve görünür durumda
                std::vector<Ability> emptyAbilities;
                hud.selectedPanel.updateQueue({}, 0.0f);
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

            hud.selectedPanel.updateSelection(
                entity->getName(),
                (int)entity->health, entity->getMaxHealth(),
                entity->getIcon(),
                entity->getAbilities()
            );
        }

        // 3. SEÇÝM KUTUSU BAÞLAT
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

        if (!ent->getIsAlive()) continue;

        sf::Vector2f diff = ent->getPosition() - worldPos;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < 20.0f * 20.0f) { // 20px yarýçap
            clickedEnemyUnit = ent;
            break;
        }
    }

    // --- SAVAÞ SÝSÝ KONTROLÜ ---
    // Eðer týklanan düþman askeri þu an görünmüyorsa (isVisible == false), ona týklanmamýþ sayarýz.
    // Böylece oyuncu karanlýða týkladýðýnda oraya "Yürüme" emri verir, görünmez adama saldýrmaz.
    if (clickedEnemyUnit && m_fogOfWar) {
        if (!m_fogOfWar->isVisible(clickedEnemyUnit->getPosition().x, clickedEnemyUnit->getPosition().y)) {
            clickedEnemyUnit = nullptr;
        }
    }
    // Not: Binalar için de benzer kontrol yapýlabilir ama bina "Unexplored" (Simsiyah) alandaysa zaten týklayan kiþi göremez.
    // "Explored" (Gri) alandaysa yerini bildiði için saldýrabilir. Bu yüzden bina kontrolü eklemiyoruz.

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

            for (const auto& entity : localPlayer.getEntities()) {
                if (entity->getIsAlive() && !entity->isSelected) {
                    reservedTiles.insert(entity->getGridPoint());
                }
            }

            for (auto& entity : localPlayer.selected_entities) {
                if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                    if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                        soldier->setForceMove();
                    }

                    Point specificGridTarget = PathFinder::findClosestFreeTile(
                        baseTarget, levelData, mapManager.getWidth(), mapManager.getHeight(), reservedTiles
                    );
                    reservedTiles.insert(specificGridTarget);

                    std::vector<Point> gridPath = PathFinder::findPath(
                        unit->getGridPoint(), specificGridTarget, levelData, mapManager.getWidth(), mapManager.getHeight()
                    );

                    if (gridPath.empty()) {
                        if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                            soldier->state = SoldierState::Idle;
                        }
                    }
                    else {
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

        // --- SAVAÞ SÝSÝ GÜNCELLEMESÝ ---
        if (m_fogOfWar) {
            m_fogOfWar->update(localPlayer.getEntities());
        }

        handleInput(dt);
        const auto& levelData = mapManager.getLevelData();
        int mapW = mapManager.getWidth();
        int mapH = mapManager.getHeight();
        const auto& allBuildings = mapManager.getBuildings();

        // --- ENTITY GÜNCELLEMELERÝ ---
        for (auto& entity : localPlayer.getEntities()) {

            // 1. Fiziksel Hareket
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                unit->update(dt, levelData, mapW, mapH);
            }

            // 2. KÖYLÜ MANTIÐI
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                // ESKÝ HALÝ: villager->updateVillager(dt, allBuildings, localPlayer);

                // YENÝ HALÝ (Harita verilerini ekliyoruz):
                villager->updateVillager(dt, allBuildings, localPlayer, levelData, mapW, mapH);
            }

            // 3. ASKER MANTIÐI
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->updateSoldier(dt, enemyPlayer.getEntities());
            }

            // 4. Binalar
            if (auto barracks = std::dynamic_pointer_cast<Barracks>(entity)) {
                ProductionSystem::update(localPlayer, *barracks, dt, mapManager);
            }
            if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity)) {
                ProductionSystem::updateTC(localPlayer, *tc, dt, mapManager);
            }
        }

        // --- KAYNAK SÝSTEMÝ ---
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
        if (!localPlayer.selected_entities.empty()) {
            auto entity = localPlayer.selected_entities[0];
            if (entity->getIsAlive()) {
                hud.selectedPanel.setVisible(true);
                hud.selectedPanel.updateHealth((int)entity->health, entity->getMaxHealth());

                //Yeni ekledim Test
                bool showQueue = false;

                if (entity->getTeam() == localPlayer.getTeamColor()) {

                    // 2. Kural: Seçilen birim BÝNA MI?
                    if (auto building = std::dynamic_pointer_cast<Building>(entity)) {
                        hud.selectedPanel.updateQueue(
                            building->getProductionQueueIcons(),
                            building->getProductionProgress()
                        );
                        showQueue = true; // Evet, kuyruðu göster!
                    }
                }
                if (!showQueue) {
                    hud.selectedPanel.updateQueue({}, 0.0f);
                }
                //Deneme buraya kadardý
            }
            else {
                hud.selectedPanel.setVisible(false);
                localPlayer.selected_entities.clear();
            }
        }
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

        gameDuration += dt;

        for (auto& building : mapManager.getBuildings()) {
            if (auto barracks = std::dynamic_pointer_cast<Barracks>(building)) {

                if (barracks->getTeam() == TeamColors::Red) {
                    barracks->updateTimer(dt);
                    if (gameDuration < 30.0f) continue;

                    if (!barracks->getIsProducing()) {
                        barracks->startTraining(SoldierTypes::Barbarian, 10.0f);
                    }
                    ProductionSystem::update(enemyPlayer, *barracks, dt, mapManager);
                }
            }
        }

        for (auto& entity : enemyPlayer.getEntities()) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->update(dt, levelData, mapW, mapH);
                soldier->updateSoldier(dt, localPlayer.getEntities());
            }
        }

        mapManager.removeDeadBuildings();
        localPlayer.removeDeadEntities();
        enemyPlayer.removeDeadEntities();

        // NOT: Render fonksiyonlarý Game::render içine taþýndý.

        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2], localPlayer);
    }
}

void Game::render() {
    window.clear();
    window.setView(camera);

    if (stateManager.getState() == GameState::Playing) {
        mapManager.draw(window);

        // 1. Kendi Birimlerimiz (Her zaman çizilir)
        localPlayer.renderEntities(window);
        for (auto& entity : localPlayer.getEntities()) {
            if (entity->getIsAlive()) entity->renderEffects(window);
        }

        // 2. Düþman Birimleri (Sis Kontrollü)
        for (auto& entity : enemyPlayer.getEntities()) {
            if (!entity->getIsAlive()) continue;

            bool isVisible = true;
            if (m_fogOfWar) isVisible = m_fogOfWar->isVisible(entity->getPosition().x, entity->getPosition().y);

            // Bina ise: Her zaman çiz (Sis üzerine binecek ve örtecek)
            if (std::dynamic_pointer_cast<Building>(entity)) {
                entity->render(window);
                // Efektleri sadece görünürse çizmek daha þýk olur
                if (isVisible) entity->renderEffects(window);
            }
            // Asker ise: SADECE Görünürse çiz
            else if (isVisible) {
                entity->render(window);
                entity->renderEffects(window);
            }
        }

        // 3. Savaþ Sisi (En üst katman)
        if (m_fogOfWar) m_fogOfWar->draw(window);

        // 4. Seçim Kutusu ve Hayalet Bina
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

    drawWarning(window);

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

    float widthInTiles = 4.0f;
    float heightInTiles = 4.0f;

    if (type == BuildTypes::House) {
        widthInTiles = 2.0f;
        heightInTiles = 2.0f;
    }
    else if (type == BuildTypes::TownCenter) {
        widthInTiles = 6.0f;
        heightInTiles = 6.0f;
    }

    float targetWidth = widthInTiles * mapManager.getTileSize();
    float targetHeight = heightInTiles * mapManager.getTileSize();

    sf::Vector2u texSize = tex.getSize();
    ghostBuildingSprite.setScale(targetWidth / texSize.x, targetHeight / texSize.y);
    ghostGridRect.setSize(sf::Vector2f(targetWidth, targetHeight));

    std::cout << "[GAME] Insaat modu aktif (Boyut: " << widthInTiles << "x" << heightInTiles << ")\n";
}

// Game.cpp

void Game::showWarning(const std::string& message) {
    warningMsg = message;
    warningClock.restart(); // Sayacý sýfýrla
    isWarningActive = true;
}

void Game::drawWarning(sf::RenderWindow& window) {
    // 3 saniye (veya senin belirlediðin süre) geçtiyse çizme
    if (!isWarningActive || warningClock.getElapsedTime().asSeconds() > 2.5f) {
        isWarningActive = false;
        return;
    }

    // FONTU ASSET MANAGER'DAN AL (En önemli düzeltme burasý!)
    // Eðer AssetManager kullanmýyorsan fontu Game sýnýfýnýn üyesi yapmalýsýn.
    sf::Font& font = AssetManager::getFont("assets/fonts/arial.ttf");

    sf::Text warnText(warningMsg, font, 24); // Yazýyý biraz büyüttüm
    warnText.setFillColor(sf::Color::Red);
    warnText.setOutlineColor(sf::Color::Black);
    warnText.setOutlineThickness(1.5f); // Okunabilirlik için kontur

    // Ortala ve Konumlandýr (SelectedObjectPanel'in üstüne denk gelecek þekilde)
    sf::FloatRect textRect = warnText.getLocalBounds();
    warnText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

    // Ekranýn alt orta kýsmý (HUD'un hemen üstü)
    float screenX = window.getSize().x / 2.0f;
    float screenY = window.getSize().y - 200.0f;
    warnText.setPosition(screenX, screenY);

    // Arka plan kutusu
    sf::RectangleShape bg(sf::Vector2f(textRect.width + 20, textRect.height + 10));
    bg.setFillColor(sf::Color(0, 0, 0, 180));
    bg.setOrigin(bg.getSize().x / 2.0f, bg.getSize().y / 2.0f);
    bg.setPosition(screenX, screenY);

    window.draw(bg);
    window.draw(warnText);
}

void Game::cancelBuildMode() {
    isInBuildMode = false;
    std::cout << "[GAME] Insaat modu iptal.\n";
}