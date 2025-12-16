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

#include "UI/AssetManager.h"

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

    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150)); // Yarý saydam beyaz
    ghostGridRect.setSize(sf::Vector2f(32, 32)); // TileSize
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);
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

        uiManager.handleEvent(event);
        hud.handleEvent(event);

        if (event.type == sf::Event::Closed)
            window.close();

        // --- ÝNÞAAT MODU TUÞ KONTROLLERÝ ---
        if (stateManager.getState() == GameState::Playing) {

            // Klavye Kýsayollarý
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::H) {
                    // H tuþuna basýnca Ev yapma moduna geç
                    // Texture yoksa bile çalýþýr (Beyaz kare çýkar)
                    enterBuildMode(BuildTypes::House, "assets/icons/house_icon.jpg");
                }
                if (event.key.code == sf::Keyboard::Escape) {
                    cancelBuildMode();
                }
            }

            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

            // UI Korumasý: Mouse panelin üzerindeyse haritaya týklama
            bool isMouseOnPanel = (pixelPos.y > 600);
            bool isMouseOnTopBar = (pixelPos.y < 50);

            // Eðer UI üzerindeysek ve týklýyorsak, harita kodlarýný çalýþtýrma
            if ((event.type == sf::Event::MouseButtonPressed) && (isMouseOnTopBar || isMouseOnPanel)) {
                continue;
            }

            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // Grid Koordinatlarýný Hesapla
            int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

            // --- TIKLAMA ÝÞLEMLERÝ ---
            if (event.type == sf::Event::MouseButtonPressed) {

                // --- SAÐ TIK (ÝPTAL VEYA HAREKET) ---
                if (event.mouseButton.button == sf::Mouse::Right) {
                    if (isInBuildMode) {
                        cancelBuildMode(); // Ýnþaat modundaysak iptal et
                    }
                    else {
                        // --- MEVCUT HAREKET KODLARI (Aynen koruyoruz) ---
                        if (gridX >= 0 && gridX < mapManager.getWidth() &&
                            gridY >= 0 && gridY < mapManager.getHeight()) {

                            Point baseTarget = { gridX, gridY };
                            std::set<Point> reservedTiles;
                            const auto& levelData = mapManager.getLevelData();

                            // 1. Yer Kaplayanlarý Bul
                            for (const auto& entity : localPlayer.getEntities()) {
                                if (auto u = std::dynamic_pointer_cast<Unit>(entity)) {
                                    if (!u->isSelected) {
                                        reservedTiles.insert(u->getGridPoint());
                                    }
                                }
                            }

                            // 2. Seçili askerlere hedef daðýt
                            for (auto& entity : localPlayer.selected_entities) {
                                if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {
                                    Point specificGridTarget = PathFinder::findClosestFreeTile(
                                        baseTarget, levelData, mapManager.getWidth(), mapManager.getHeight(), reservedTiles
                                    );
                                    reservedTiles.insert(specificGridTarget);

                                    // A* Yol Bulma
                                    std::vector<Point> gridPath = PathFinder::findPath(
                                        unit->getGridPoint(), specificGridTarget, levelData, mapManager.getWidth(), mapManager.getHeight()
                                    );

                                    // Grid -> Pixel Çevirme
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

                // --- SOL TIK (SEÇÝM VEYA ÝNÞAAT) ---
                else if (event.mouseButton.button == sf::Mouse::Left) {
                    if (isInBuildMode) {
                        // --- ÝNÞAAT YAP ---
                        int cost = GameRules::Cost_House_Wood;

                        // Kaynak Yeterli mi?
                        if (localPlayer.getResources()[0] >= cost) {

                            // ResourceManager parametresini sildik (Geçici olarak)
                            // mapManager.tryPlaceBuilding fonksiyonu son halinde resMgr istiyor mu kontrol etmeliyiz.
                            // Eðer istiyorsa ve Player'dan eriþemiyorsak, þimdilik o parametreyi MapManager.h/cpp'den kaldýr veya dummy gönder.
                            // Aþaðýdaki kod MapManager'ýn parametre istemeyen haline göredir:

                            // NOT: Eðer MapManager::tryPlaceBuilding hata verirse parametreleri kontrol et.
                            // Þimdilik kaynak düþme iþini burada yapýyoruz:

                            // (MapManager'da tryPlaceBuilding fonksiyonunu çaðýrýrken ResourceManager istiyorsa orayý düzeltmemiz lazým, 
                            // þimdilik parametresiz varsayýyorum veya dummy bir çözüm üretebiliriz)

                            // Geçici Çözüm: ResourceManager referansý yerine kaynak kontrolünü burada yaptýk.
                            // MapManager.cpp'deki tryPlaceBuilding'den "ResourceManager& resMgr" parametresini silmeni öneririm 
                            // çünkü kaynak düþümünü zaten Game.cpp'de yapýyoruz.

                            // Eðer tryPlaceBuilding parametre istiyorsa hatayý düzeltmek için MapManager.cpp'ye git ve o parametreyi sil.
                            if (mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType, localPlayer.playerResources)) {

                                localPlayer.addWood(-cost);
                                std::cout << "[GAME] Bina insa edildi!\n";

                                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                                    cancelBuildMode();
                                }
                            }
                            else {
                                std::cout << "[GAME] Buraya insa edilemez!\n";
                            }
                        }
                        else {
                            std::cout << "[GAME] Yetersiz Kaynak!\n";
                            cancelBuildMode(); // Parasý yoksa modu kapat
                        }
                    }
                    else {
                        // --- ASKER SEÇÝMÝ ---
                        localPlayer.selectUnit(window, camera);
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

        // --- ÝNÞAAT MODU GÖRSELÝ ---
        if (isInBuildMode) {
            // Mouse pozisyonunu al
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // Grid'e yapýþtýr (Snap to Grid)
            int gx = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gy = static_cast<int>(worldPos.y / mapManager.getTileSize());

            float snapX = gx * mapManager.getTileSize();
            float snapY = gy * mapManager.getTileSize();

            ghostBuildingSprite.setPosition(snapX, snapY);
            ghostGridRect.setPosition(snapX, snapY);

            // Harita sýnýrlarýnda mý?
            bool isValid = (gx >= 0 && gx < mapManager.getWidth() && gy >= 0 && gy < mapManager.getHeight());
            // Buraya mapManager.isBuildingAt(gx, gy) kontrolü de eklenebilir

            if (isValid) ghostBuildingSprite.setColor(sf::Color(0, 255, 0, 150)); // Yeþil
            else ghostBuildingSprite.setColor(sf::Color(255, 0, 0, 150)); // Kýrmýzý

            window.draw(ghostBuildingSprite);
            window.draw(ghostGridRect);
        }
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

void Game::enterBuildMode(BuildTypes type, const std::string& textureName) {
    isInBuildMode = true;
    pendingBuildingType = type;

    // Texture'ý ResourceManager veya AssetManager'dan almalýsýn. 
    // Þimdilik test için AssetManager kullanýyoruz ama normalde Player resources'dan almalý.
    ghostBuildingSprite.setTexture(AssetManager::getTexture(textureName)); // "tileset.png" veya özel bina resmi

    // Texture'ýn boyutuyla oynayabiliriz veya tileset kullanýyorsak TextureRect ayarlamalýyýz.
    // Þimdilik basit tutalým, tileset'in tamamýný deðil, ilgili kýsmýný göstermeli ama 
    // senin sisteminde binalar ayrý class olduðu için þimdilik varsayýlan bir ikon koyalým.
    // EÐER tileset kullanýyorsan ve bina oradaysa:
    // ghostBuildingSprite.setTextureRect(...); 

    std::cout << "[GAME] Insaat modu aktif: " << (int)type << "\n";
}

void Game::cancelBuildMode() {
    isInBuildMode = false;
    std::cout << "[GAME] Insaat modu iptal.\n";
}