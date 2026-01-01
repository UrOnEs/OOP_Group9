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
#include "Network/NetServer.h" // NetServer fonksiyonlarını (setOnPacket) kullanmak için gerekli
#include "Network/NetClient.h" // NetClient fonksiyonlarını kullanmak için gerekli

// --- YENİ EKLENEN INCLUDE ---
#include "Map/FogOfWar.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> 

#include "UI/AssetManager.h"
#include "Game/GameRules.h"

//bool GameRules::DebugMode = false;

Game::Game(bool isHost, std::string serverIp, int playerIndex, int totalPlayerCount)
    : mapManager(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize),
    m_isHost(isHost),     // Kaydet
    m_serverIp(serverIp),
    m_playerIndex(playerIndex),
    m_totalPlayerCount(totalPlayerCount), // Kaydet
    m_connectedClientCount(0)

{
    // 1. Pencere ve Görünüm Ayarları
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Empires of Ages - RTS", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    camera.setSize(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height));
    camera.setCenter(desktopMode.width / 2.0f, desktopMode.height / 2.0f);

    // 2. Arayüz ve Ağ Sistemlerini Başlat
    // NOT: hud.init() artık startMatch içinde çağrılıyor çünkü harita verisine ihtiyacı var.

    initUI();
    initNetwork(); // Bu fonksiyon ileride startMatch'i tetikleyecek

    // 3. Savaş Sisi (Harita içeriğinden bağımsızdır, güvenle oluşturulabilir)
    m_fogOfWar = std::make_unique<FogOfWar>(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize);

    // 4. İnşaat ve Seçim Görselleri (UI)
    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));

    ghostGridRect.setSize(sf::Vector2f(GameRules::TileSize, GameRules::TileSize));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);

    // 5. Arka Plan Müziği
    if (bgMusic.openFromFile("assets/sounds/background_music.ogg")) {
        bgMusic.setLoop(true);
        bgMusic.setVolume(GameRules::BackgroundMusicVolume);
        bgMusic.play();
    }

    // --- DİKKAT ---
    // mapManager.initialize(), clearArea(), addEntity() gibi harita ve birim
    // oluşturma kodlarının tamamı buradan SİLİNDİ ve startMatch() fonksiyonuna taşındı.
    // Bu sayede oyun, sunucudan seed gelmeden haritaya dokunmaz ve çökmez.
}

void Game::startMatch(unsigned int seed) {
    std::cout << "[GAME] Mac baslatiliyor. Seed: " << seed << " | My Index: " << m_playerIndex << std::endl;

    // A. Haritayı Seed ile Oluştur
    mapManager.initialize(seed);

    // --- MINIMAP VE HUD BAŞLATMA (YENİ) ---
    // Harita verisi oluştuğu için artık HUD'u ve Minimap'i başlatabiliriz.
    sf::Vector2u winSize = window.getSize();
    hud.init(winSize.x, winSize.y,
        GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize,
        mapManager.getLevelData());
    // --------------------------------------

    // B. Önceki Entityleri Temizle (Varsa)
    localPlayer.entities.clear();
    localPlayer.selected_entities.clear();
    enemyPlayer.entities.clear();
    enemyPlayer.selected_entities.clear();
    // ResourceManager sıfırlama vb. gerekirse buraya eklersin.

    // C. Başlangıç Binalarını Yerleştir (Constructor'dan aldığımız kodlar)
    // ====================================================================
    // C. SPAWN NOKTALARINA GÖRE YERLEÞTÝRME
    // GameRules.h içinde 4 adet spawn noktasý tanýmlý varsayýyoruz.
    for (int i = 0; i < 4; ++i) {
        sf::Vector2i spawnGrid = GameRules::SpawnPoints[i];

        // Bölgeyi temizle (Aðaç vs. varsa kaldýrsýn)
        // Merkez binanýn sýðacaðý kadar alaný temizle (6x6 town center için biraz pay býrakýyoruz)
        mapManager.clearArea(spawnGrid.x - 3, spawnGrid.y - 3, 10, 10);

        // Eðer bu index BENÝM ise -> LocalPlayer'a ekle
        if (i == m_playerIndex) {
            localPlayer.setTeamColor((TeamColors)i); // Rengini ayarla

            // 1. Town Center
            std::shared_ptr<Building> myTC = mapManager.tryPlaceBuilding(spawnGrid.x, spawnGrid.y, BuildTypes::TownCenter);
            if (myTC) {
                myTC->setTeam((TeamColors)i);
                localPlayer.addEntity(myTC);

                // 2. Villager (TC'nin biraz yanýna)
                std::shared_ptr<Villager> myVil = std::make_shared<Villager>();
                sf::Vector2f spawnPos = myTC->getPosition();
                spawnPos.y += 120.0f; // Biraz aþaðýya koy
                myVil->setPosition(spawnPos);
                myVil->setTeam((TeamColors)i);
                localPlayer.addEntity(myVil);

                // Kamerayý kendi merkezimize odakla
                camera.setCenter(myTC->getPosition());
            }
        }
        // Eðer bu index BAÞKASI ise -> EnemyPlayer'a ekle
        else {
            // 1. Enemy Town Center
            std::shared_ptr<Building> enemyTC = mapManager.tryPlaceBuilding(spawnGrid.x, spawnGrid.y, BuildTypes::TownCenter);
            if (enemyTC) {
                enemyTC->setTeam((TeamColors)i); // Dusmanin rengini ayarla
                enemyPlayer.addEntity(enemyTC);

                // --- EKLENEN KISIM: DÜŞMAN KÖYLÜSÜ ---
                // Tıpkı localPlayer'da olduğu gibi düşman için de başlangıç köylüsü ekliyoruz.
                std::shared_ptr<Villager> enemyVil = std::make_shared<Villager>();
                sf::Vector2f spawnPos = enemyTC->getPosition();
                spawnPos.y += 120.0f;
                enemyVil->setPosition(spawnPos);
                enemyVil->setTeam((TeamColors)i);
                enemyPlayer.addEntity(enemyVil);
                // -------------------------------------
            }
        }
    }

    // Oyunu oynanıyor moduna al
    stateManager.setState(GameState::Playing);
}

void Game::initNetwork() {
    networkManager.setLogger([](const std::string& msg) {});

    // Paket İşleyici
    auto packetHandler = [this](uint64_t id, sf::Packet& pkt) {
        if (stateManager.getState() == GameState::Playing) {
            sf::Packet copyPkt = pkt;
            sf::Uint8 cmdRaw;

            // Okuma hatası kontrolü
            if (!(copyPkt >> cmdRaw)) return;

            NetCommand cmd = (NetCommand)cmdRaw;

            

            // --- TRAIN UNIT ---
            if (cmd == NetCommand::TrainUnit) {
                int gx, gy, uType;
                if (copyPkt >> gx >> gy >> uType) {

                    if (m_isHost && id != 0) {
                        // Temiz bir paket oluştur ve diğerlerine gönder
                        sf::Packet forwardPacket;
                        forwardPacket << (sf::Uint8)NetCommand::TrainUnit << gx << gy << uType;
                        networkManager.server()->sendToAllExcept(id, forwardPacket);
                    }

                    auto building = mapManager.getBuildingAt(gx, gy);

                    // RENK KONTROLÜNÜ GEVŞETTİK: "Benim değilse düşmanındır"
                    if (building && building->getTeam() != localPlayer.getTeamColor()) {

                        // İŞLEMİN GARANTİ ÇALIŞMASI İÇİN GEÇİCİ KAYNAK
                        enemyPlayer.addUnitLimit(50);
                        enemyPlayer.addFood(2000);
                        enemyPlayer.addWood(2000);
                        enemyPlayer.addGold(2000);
                        enemyPlayer.addStone(2000);

                        if (uType == 1 && std::dynamic_pointer_cast<TownCenter>(building)) {
                            ProductionSystem::startVillagerProduction(enemyPlayer, *std::dynamic_pointer_cast<TownCenter>(building));
                            std::cout << "[AG] Rakip (ID: " << id << ") koylu uretiyor.\n";
                        }
                        else if (std::dynamic_pointer_cast<Barracks>(building)) {
                            auto bar = std::dynamic_pointer_cast<Barracks>(building);
                            SoldierTypes type = SoldierTypes::Barbarian;
                            if (uType == 3) type = SoldierTypes::Archer;
                            if (uType == 4) type = SoldierTypes::Wizard;
                            ProductionSystem::startProduction(enemyPlayer, *bar, type);
                            std::cout << "[AG] Rakip (ID: " << id << ") asker uretiyor.\n";
                        }
                    }
                }
            }
            // --- PLACE BUILDING ---
            else if (cmd == NetCommand::PlaceBuilding) {
                int gx, gy, bTypeInt;
                if (copyPkt >> gx >> gy >> bTypeInt) {

                    if (m_isHost && id != 0) {
                        sf::Packet forwardPacket;
                        forwardPacket << (sf::Uint8)NetCommand::PlaceBuilding << gx << gy << bTypeInt;
                        networkManager.server()->sendToAllExcept(id, forwardPacket);
                    }

                    BuildTypes type = (BuildTypes)bTypeInt;

                    // Bina daha önce kurulmamışsa kur
                    if (mapManager.getBuildingAt(gx, gy) == nullptr) {
                        std::shared_ptr<Building> enemyBuilding = mapManager.tryPlaceBuilding(gx, gy, type);
                        if (enemyBuilding) {
                            // Düşman rengini ata (Basit mantık: Bizim değilse kırmızı/mavi yap)
                            TeamColors targetColor = (localPlayer.getTeamColor() == TeamColors::Blue) ? TeamColors::Red : TeamColors::Blue;
                            enemyBuilding->setTeam(targetColor);

                            enemyBuilding->isConstructed = false;
                            enemyBuilding->health = 1.0f;
                            enemyPlayer.addEntity(enemyBuilding);
                            std::cout << "[AG] Rakip bina insa etti: " << gx << "," << gy << "\n";
                        }
                    }
                }
            }
        }
        else {
            if (lobbyManager) lobbyManager->handleIncomingPacket(id, pkt);
        }
        };

    if (m_isHost) {
        // --- HOST ---
        if (networkManager.startServer(54000)) {

            // --- EKSİK OLAN SATIR BURASIYDI ---
            lobbyManager = std::make_unique<LobbyManager>(&networkManager, true);
            // ----------------------------------

            networkManager.server()->setOnPacket(packetHandler);
            lobbyManager->start(1, "HostPlayer");
            lobbyManager->toggleReady(true);

            // Bekleme Mantığı
            if (m_totalPlayerCount == 1) {
                std::cout << "[GAME] Tek kisilik oyun. Hemen baslatiliyor.\n";
                m_startGameTimer = 0.5f; // 100ms sonra başlat
            }
            else {
                std::cout << "[GAME] Toplam " << m_totalPlayerCount << " oyuncu bekleniyor...\n";
                networkManager.server()->setOnClientConnected([this](uint64_t clientId) {
                    m_connectedClientCount++;
                    std::cout << "[GAME] Oyuncu (" << clientId << ") baglandi. Durum: "
                        << m_connectedClientCount << "/" << (m_totalPlayerCount - 1) << "\n";

                    if (m_connectedClientCount >= (m_totalPlayerCount - 1)) {
                        std::cout << "[GAME] HERKES HAZIR! Mac baslatiliyor...\n";
                        m_startGameTimer = 0.1f;
                    }
                    });
            }
        }
        else {
            std::cerr << "[GAME] HATA: Sunucu baslatilamadi!" << std::endl;
        }
    }
    else {
        // --- CLIENT ---
        std::cout << "[GAME] Sunucuya baglaniliyor: " << m_serverIp << "...\n";
        if (networkManager.startClient(m_serverIp, 54000)) {
            lobbyManager = std::make_unique<LobbyManager>(&networkManager, false);
            networkManager.client()->setOnPacket([packetHandler](sf::Packet& pkt) { packetHandler(0, pkt); });
            lobbyManager->start(0, "ClientPlayer");
            lobbyManager->toggleReady(true);

            sf::Packet dummy; dummy << (sf::Uint8)NetCommand::None;
            networkManager.client()->sendReliable(dummy);
            std::cout << "[GAME] Client baglandi. Oyun baslamasi bekleniyor...\n";
        }
        else {
            std::cerr << "[GAME] HATA: Baglanilamadi!\n";
        }
    }

    if (lobbyManager) {
        lobbyManager->setOnGameStart([this]() {
            unsigned int seed = lobbyManager->getGameSeed();
            this->startMatch(seed);
            });
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

        // --- HATA BURADAYDI ---
        // hud.handleEvent(event); // <-- BU SATIR BURADA OLDUĞU İÇİN ÇÖKÜYOR OLABİLİR

        if (event.type == sf::Event::Closed) {
            window.close();
        }

        // SADECE OYUN OYNANIYORSA HUD VE OYUN GİRDİLERİNİ İŞLE
        if (stateManager.getState() == GameState::Playing) {

            // --- BURAYA TAŞIYORUZ ---
            // Artık güvenli, çünkü startMatch çalıştı ve HUD yüklendi.
            hud.handleEvent(event);
            // ------------------------

            // 1. KLAVYE GİRDİLERİ
            if (event.type == sf::Event::KeyPressed) {
                handleKeyboardInput(event);
            }

            // 2. MOUSE GİRDİLERİ
            if (event.type == sf::Event::MouseButtonPressed ||
                event.type == sf::Event::MouseButtonReleased ||
                event.type == sf::Event::MouseMoved) {
                handleMouseInput(event);
            }
        }
    }
}

// ======================================================================================
//                                  KLAVYE KONTROLLERİ
// ======================================================================================

void Game::handleKeyboardInput(const sf::Event& event) {
    // Kısayollar
    if (event.key.code == sf::Keyboard::H) enterBuildMode(BuildTypes::House, "assets/buildings/house.png");
    if (event.key.code == sf::Keyboard::B) enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png");
    if (event.key.code == sf::Keyboard::C) enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png");
    if (event.key.code == sf::Keyboard::M) enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png");

    // İptal / Çıkış
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

    // D: DESTROY (Yıkma)
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

    if (event.type == sf::Event::KeyPressed) {
        // Toggle Dev Mode: Ctrl + Shift + D
        if (event.key.code == sf::Keyboard::D &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {

            isDevMode = !isDevMode; // Toggle on/off
            GameRules::DebugMode = !GameRules::DebugMode;

            if (isDevMode) {
                std::cout << "[DEV MODE] ON - God Mode Active!\n";
                localPlayer.addWood(9999);
                localPlayer.addFood(9999);
                localPlayer.addGold(9999);
                localPlayer.addStone(9999);

                localPlayer.addUnitLimit(50);
            }
            else {
                std::cout << "[DEV MODE] OFF - Back to reality.\n";
            }
        }
    }
}

// ======================================================================================
//                                  MOUSE KONTROLLERİ
// ======================================================================================

void Game::handleMouseInput(const sf::Event& event) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

    // --- MINIMAP KONTROLÜ (YENİ) ---
    // Sol tık basılıyken veya tıklandığında minimap'e bak
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f newCamPos;
        // Eğer minimap'e tıklandıysa:
        if (hud.minimap.handleClick(pixelPos, newCamPos)) {
            camera.setCenter(newCamPos);
            return; // Başka işlem (seçim vs.) yapma
        }
    }
    // -------------------------------

    // UI üzerindeyse oyun dünyasına tıklamayı engelle
    if (event.type == sf::Event::MouseButtonPressed && hud.isMouseOverUI(pixelPos)) return;

    // --- SOL TIK ---
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        onLeftClick(worldPos, pixelPos);
    }

    // --- SAĞ TIK ---
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        onRightClick(worldPos);
    }

    // --- SEÇİM KUTUSU GÜNCELLEME (Mouse Sürükleme) ---
    else if (event.type == sf::Event::MouseMoved && isSelecting) {
        selectionBox.setSize(worldPos - selectionStartPos);
    }

    // --- SEÇİM BİTİRME (Mouse Bırakma) ---
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isSelecting) {
            isSelecting = false;
            bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

            // Nokta mı yoksa Kutu mu?
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

                //Productiondan gelen output nasıl değerlendirilecek
                auto handleProductionResult = [this](ProductionResult result) {
                    switch (result) {
                    case ProductionResult::Success:
                        // Başarılı (Ses çalınabilir)
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
                            // 1. Önce KENDİ oyunumuzda işlemi yap
                            ProductionResult res = ProductionSystem::startVillagerProduction(localPlayer, *tc);
                            handleProductionResult(res);

                            // 2. Başarılıysa AĞA GÖNDER
                            if (res == ProductionResult::Success) {
                                // Not: UnitType ID'leri için bir standart belirlemelisin. Örn: 1=Köylü, 2=Barbar...
                                this->sendTrainCommand(tc->getGridPoint().x, tc->getGridPoint().y, 1);
                            }
                                });
                    }
                    else if (ab.getId() == 11) { // Barbar
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Barbarian);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(b->getGridPoint().x, b->getGridPoint().y, 2); // 2 = Barbar
                            }
                                });
                    }
                    else if (ab.getId() == 12) { // Okçu
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Archer);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(b->getGridPoint().x, b->getGridPoint().y, 3); // 3 = okcu
                            }
                                });
                    }
                    else if (ab.getId() == 13) { // Wizard
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Wizard);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(b->getGridPoint().x, b->getGridPoint().y, 4); // 4 = buyucu
                            }
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
//                                  SOL TIK (İNŞAAT / SEÇİM)
// ======================================================================================

void Game::onLeftClick(const sf::Vector2f& worldPos, const sf::Vector2i& pixelPos) {
    int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
    int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

    if (isInBuildMode) {
        // --- İNŞAAT YAPMA ---
        GameRules::Cost cost = GameRules::getBuildingCost(pendingBuildingType);

        bool canAfford = localPlayer.getResources()[0] >= cost.wood &&
            localPlayer.getResources()[1] >= cost.gold &&
            localPlayer.getResources()[2] >= cost.stone &&
            localPlayer.getResources()[3] >= cost.food;

        if (canAfford) {
            std::shared_ptr<Building> placed = mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType);
            if (placed) {
                // --- İNŞAAT SİSTEMİ BAŞLANGICI ---
                placed->isConstructed = false; // Henüz bitmedi
                placed->health = 1.0f;         // Canı 1 ile başlasın (Temel atıldı)
                // ---------------------------------

                localPlayer.addEntity(placed);

                localPlayer.addWood(-cost.wood);
                localPlayer.addGold(-cost.gold);
                localPlayer.addStone(-cost.stone);
                localPlayer.addFood(-cost.food);

                sendBuildCommand(gridX, gridY, (int)pendingBuildingType);

                std::cout << "[GAME] Temel atildi! Insaat bekliyor.\n";

                // Eğer BİR KÖYLÜ SEÇİLİYSE, otomatik inşaata başlasın (Kullanıcı Dostu Özellik)
                if (localPlayer.selected_entities.size() == 1) {
                    if (auto vil = std::dynamic_pointer_cast<Villager>(localPlayer.selected_entities[0])) {
                        vil->startBuilding(placed);
                    }
                }

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

        // 1. Önce KENDİ birimlerimizi seçmeyi dene
        localPlayer.selectUnit(window, camera, shift);

        // 2. Eğer bizden kimse seçilmediyse (veya boşluğa tıklandıysa), DÜŞMANLARA bak
        if (localPlayer.selected_entities.empty()) {
            enemyPlayer.selectUnit(window, camera, false); // Düşman için shift yok

            if (!enemyPlayer.selected_entities.empty()) {
                auto entity = enemyPlayer.selected_entities[0];

                // DÜZELTME: Eğer düşman görünmezse (sis içindeyse) seçimi iptal et
                if (m_fogOfWar && !m_fogOfWar->isVisible(entity->getPosition().x, entity->getPosition().y)) {
                    // Bina değilse ve görünmüyorsa seçme. (Binalar explored alanda görülebilir, bu basit kontrol şimdilik yeterli)
                    if (!std::dynamic_pointer_cast<Building>(entity)) {
                        enemyPlayer.selected_entities.clear();
                        hud.selectedPanel.setVisible(false);
                        return; // Fonksiyondan çık
                    }
                }

                // Düşman seçildi ve görünür durumda
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
            // Bizim birimimiz seçildiyse düşman seçimini temizle
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

        // 3. SEÇİM KUTUSU BAŞLAT
        isSelecting = true;
        selectionStartPos = worldPos;
        selectionBox.setPosition(selectionStartPos);
        selectionBox.setSize(sf::Vector2f(0, 0));
    }
}

// ======================================================================================
//                                  SAĞ TIK (HAREKET / SALDIRI / HASAT)
// ======================================================================================

void Game::onRightClick(const sf::Vector2f& worldPos) {
    if (isInBuildMode) {
        cancelBuildMode();
        return;
    }

    int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
    int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

    // 1. Tıklanan şey bir BİNA mı?
    std::shared_ptr<Building> clickedBuilding = mapManager.getBuildingAt(gridX, gridY);
    auto resGen = std::dynamic_pointer_cast<ResourceGenerator>(clickedBuilding);

    // 2. Tıklanan sey bir UNIT mi? (Düşman kontrolü)
    std::shared_ptr<Entity> clickedEnemyUnit = nullptr;
    for (auto& ent : enemyPlayer.getEntities()) {

        if (!ent->getIsAlive()) continue;

        sf::Vector2f diff = ent->getPosition() - worldPos;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < 20.0f * 20.0f) { // 20px yarıçap
            clickedEnemyUnit = ent;
            break;
        }
    }

    // --- SAVAŞ SİSİ KONTROLÜ ---
    // Eğer tıklanan düşman askeri şu an görünmüyorsa (isVisible == false), ona tıklanmamış sayarız.
    // Böylece oyuncu karanlığa tıkladığında oraya "Yürüme" emri verir, görünmez adama saldırmaz.
    if (clickedEnemyUnit && m_fogOfWar) {
        if (!m_fogOfWar->isVisible(clickedEnemyUnit->getPosition().x, clickedEnemyUnit->getPosition().y)) {
            clickedEnemyUnit = nullptr;
        }
    }
    // Not: Binalar için de benzer kontrol yapılabilir ama bina "Unexplored" (Simsiyah) alandaysa zaten tıklayan kişi göremez.
    // "Explored" (Gri) alandaysa yerini bildiği için saldırabilir. Bu yüzden bina kontrolü eklemiyoruz.

    // --- KARAR MEKANİZMASI ---

    if (clickedBuilding) {
        // --- A. BİR BİNAYA TIKLANDI ---

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

        // ---------- İNŞAAT EMRİ ----------------------
        // Tıklanan bina bizim takımınsa ve henüz bitmemişse
        if (clickedBuilding->getTeam() == localPlayer.getTeamColor() && !clickedBuilding->isConstructed) {
            for (auto& entity : localPlayer.selected_entities) {
                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                    villager->startBuilding(clickedBuilding);
                }
            }
            std::cout << "[GAME] Koyluler insaata gonderildi.\n";
            return; // Saldırı koduna girmesin diye çık
        }
        // -------------------------

        // 2. Askerleri Yönet (Düşman veya Bina ise Saldır)
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
        // --- B. DÜŞMAN ASKERİNE SALDIRI ---
        std::cout << "[GAME] Dusman askerine saldiri emri!\n";
        for (auto& entity : localPlayer.selected_entities) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->setTarget(clickedEnemyUnit);
            }
        }
    }
    else {
        // --- C. BOŞ YERE TIKLANDI (FORCE MOVE / HAREKET) ---

        // 1. Köylüleri durdur
        for (auto& entity : localPlayer.selected_entities) {
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                villager->stopHarvesting();
            }
        }

        // 2. Asker Hareket Mantığı
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
    // Ağ paketlerini her zaman dinlemeliyiz (Bağlantı istekleri için)
    networkManager.update(dt); //

    if (m_isHost && m_startGameTimer > 0.0f) {
        m_startGameTimer -= dt;
        if (m_startGameTimer <= 0.0f) {
            // Süre doldu, oyunu başlat
            if (lobbyManager) {
                std::cout << "[GAME] Timer doldu, start sinyali gonderiliyor...\n";
                lobbyManager->startGame();
            }
            m_startGameTimer = -1.0f; // Sayacı kapat
        }
    }

    // --- ÖNEMLİ DEĞİŞİKLİK: Sadece oyun oynanıyorsa güncelleme yap ---
    if (stateManager.getState() == GameState::Playing) { //

        // --- SAVAŞ SİSİ GÜNCELLEMESİ ---
        if (m_fogOfWar) {
            m_fogOfWar->update(localPlayer.getEntities());
        }

        // --- MINIMAP GÜNCELLEMESİ ---
        // Bu kod eskiden dışarıdaydı, artık koruma altında.
        hud.minimap.update(
            localPlayer.getEntities(),
            enemyPlayer.getEntities(),
            camera,
            m_fogOfWar.get()
        );

        handleInput(dt);
        const auto& levelData = mapManager.getLevelData();
        int mapW = mapManager.getWidth();
        int mapH = mapManager.getHeight();
        const auto& allBuildings = mapManager.getBuildings();

        // --- ENTITY GÜNCELLEMELERİ ---
        for (auto& entity : localPlayer.getEntities()) {
            if (auto u = std::dynamic_pointer_cast<Unit>(entity)) u->update(dt, levelData, mapW, mapH);
            if (auto v = std::dynamic_pointer_cast<Villager>(entity)) v->updateVillager(dt, allBuildings, localPlayer, levelData, mapW, mapH);
            if (auto s = std::dynamic_pointer_cast<Soldier>(entity)) s->updateSoldier(dt, enemyPlayer.getEntities());
            if (auto b = std::dynamic_pointer_cast<Barracks>(entity)) ProductionSystem::update(localPlayer, *b, dt, mapManager);
            if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity)) ProductionSystem::updateTC(localPlayer, *tc, dt, mapManager);
        }

        // --- KAYNAK SİSTEMİ ---
        for (auto& building : mapManager.getBuildings()) {
            if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {
                if (res->isWorking()) ResourceSystem::update(localPlayer, *res, dt);
            }
        }

        // --- UI SEÇİM GÜNCELLEME ---
        if (!localPlayer.selected_entities.empty()) {
            auto ent = localPlayer.selected_entities[0];
            if (ent->getIsAlive()) {
                hud.selectedPanel.setVisible(true);
                hud.selectedPanel.updateHealth((int)ent->health, ent->getMaxHealth());
                if (auto b = std::dynamic_pointer_cast<Building>(ent)) {
                    if (b->getTeam() == localPlayer.getTeamColor())
                        hud.selectedPanel.updateQueue(b->getProductionQueueIcons(), b->getProductionProgress());
                }
                else {
                    hud.selectedPanel.updateQueue({}, 0.0f);
                }
            }
            else {
                hud.selectedPanel.setVisible(false);
                localPlayer.selected_entities.clear();
            }
        }
        else if (!enemyPlayer.selected_entities.empty()) {
            auto ent = enemyPlayer.selected_entities[0];
            if (!ent->getIsAlive()) {
                hud.selectedPanel.setVisible(false);
                enemyPlayer.selected_entities.clear();
            }
        }

        // --- DÜŞMAN GÜNCELLEME ---
        for (auto& ent : enemyPlayer.getEntities()) {
            if (auto s = std::dynamic_pointer_cast<Soldier>(ent)) {
                s->update(dt, levelData, mapW, mapH);
                s->updateSoldier(dt, localPlayer.getEntities());
            }
            if (auto b = std::dynamic_pointer_cast<Barracks>(ent)) ProductionSystem::update(enemyPlayer, *b, dt, mapManager);
            if (auto tc = std::dynamic_pointer_cast<TownCenter>(ent)) ProductionSystem::updateTC(enemyPlayer, *tc, dt, mapManager);
        }

        mapManager.removeDeadBuildings();
        localPlayer.removeDeadEntities();
        enemyPlayer.removeDeadEntities();

        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2], localPlayer);
    }
}

void Game::render() {
    window.clear();
    window.setView(camera);

    if (stateManager.getState() == GameState::Playing) { //
        // --- OYUN DÜNYASI ÇİZİMİ ---
        mapManager.draw(window);

        // Kendi birimlerimiz
        localPlayer.renderEntities(window);
        for (auto& entity : localPlayer.getEntities()) {
            if (entity->getIsAlive()) entity->renderEffects(window);
        }

        // Düşman birimleri (Sis kontrollü)
        for (auto& entity : enemyPlayer.getEntities()) {
            if (!entity->getIsAlive()) continue;
            bool isVisible = true;
            if (m_fogOfWar) isVisible = m_fogOfWar->isVisible(entity->getPosition().x, entity->getPosition().y);

            if (std::dynamic_pointer_cast<Building>(entity)) {
                entity->render(window);
                if (isVisible) entity->renderEffects(window);
            }
            else if (isVisible) {
                entity->render(window);
                entity->renderEffects(window);
            }
        }

        // Savaş Sisi
        if (m_fogOfWar) m_fogOfWar->draw(window);

        // Seçim ve İnşaat
        if (isSelecting) window.draw(selectionBox);
        if (isInBuildMode) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            int gx = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gy = static_cast<int>(worldPos.y / mapManager.getTileSize());

            float snapX = gx * mapManager.getTileSize();
            float snapY = gy * mapManager.getTileSize();

            ghostBuildingSprite.setPosition(snapX, snapY);
            ghostGridRect.setPosition(snapX, snapY);
            window.draw(ghostBuildingSprite);
            window.draw(ghostGridRect);
        }
    }

    // --- UI KATMANI ---
    window.setView(window.getDefaultView());

    // Eğer oyun oynanıyorsa HUD'u çiz
    if (stateManager.getState() == GameState::Playing) { //
        hud.draw(window);
        uiManager.draw(window);
        drawWarning(window);
    }
    // Eğer oyun başlamadıysa (Bekleme Modu) Bilgi Ekranı Çiz
    else {
        // Font AssetManager'dan veya eldeki bir fonttan
        // Not: AssetManager yoksa basit bir sf::Font yüklemesi yapmanız gerekebilir.
        sf::Font& font = AssetManager::getFont("assets/fonts/arial.ttf");

        sf::Text waitText;
        waitText.setFont(font);
        waitText.setCharacterSize(24);
        waitText.setFillColor(sf::Color::White);

        if (m_isHost) {
            waitText.setString("Oyuncular Bekleniyor... (" +
                std::to_string(m_connectedClientCount) + "/" +
                std::to_string(m_totalPlayerCount - 1) + ")");
        }
        else {
            waitText.setString("Oyun Baslatma Sinyali Bekleniyor...");
        }

        // Ortala
        sf::FloatRect textRect = waitText.getLocalBounds();
        waitText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        waitText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);

        window.draw(waitText);
    }

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
    warningClock.restart(); // Sayacı sıfırla
    isWarningActive = true;
}

void Game::drawWarning(sf::RenderWindow& window) {
    // 3 saniye (veya senin belirlediğin süre) geçtiyse çizme
    if (!isWarningActive || warningClock.getElapsedTime().asSeconds() > 2.5f) {
        isWarningActive = false;
        return;
    }

    // FONTU ASSET MANAGER'DAN AL (En önemli düzeltme burası!)
    // Eğer AssetManager kullanmıyorsan fontu Game sınıfının üyesi yapmalısın.
    sf::Font& font = AssetManager::getFont("assets/fonts/arial.ttf");

    sf::Text warnText(warningMsg, font, 24); // Yazıyı biraz büyüttüm
    warnText.setFillColor(sf::Color::Red);
    warnText.setOutlineColor(sf::Color::Black);
    warnText.setOutlineThickness(1.5f); // Okunabilirlik için kontur

    // Ortala ve Konumlandır (SelectedObjectPanel'in üstüne denk gelecek şekilde)
    sf::FloatRect textRect = warnText.getLocalBounds();
    warnText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

    // Ekranın alt orta kısmı (HUD'un hemen üstü)
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

void Game::sendTrainCommand(int gridX, int gridY, int unitTypeID) {
    sf::Packet packet;
    // 1. Komut Tipi
    packet << (sf::Uint8)NetCommand::TrainUnit; // Enum'ı Uint8 olarak yaz
    // 2. Hangi Bina? (Koordinat ile bulmak en kolayıdır)
    packet << gridX << gridY;
    // 3. Hangi Birim?
    packet << unitTypeID;

    // Herkese gönder
    if (m_isHost) {
        networkManager.server()->sendToAllReliable(packet);
    }
    else {
        networkManager.client()->sendReliable(packet);
    }
}

// source/Game/Game.cpp dosyasının en altlarına ekleyin:

void Game::sendBuildCommand(int gridX, int gridY, int buildTypeID) {
    sf::Packet packet;
    // 1. Komut Tipi
    packet << (sf::Uint8)NetCommand::PlaceBuilding;
    // 2. Koordinat
    packet << gridX << gridY;
    // 3. Bina Tipi (BuildTypes enum'ını int olarak gönderiyoruz)
    packet << buildTypeID;

    // Herkese gönder
    if (m_isHost) {
        networkManager.server()->sendToAllReliable(packet);
    }
    else {
        networkManager.client()->sendReliable(packet);
    }
}