#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include "Entity System/Entity Type/Unit.h"
#include <iostream>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> // Üst üste binmeyi engellemek için

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
    testSoldier->setPosition(sf::Vector2f(500.f, 500.f));

    // 2. Türünü Belirle (Hýz ve Can deðerleri yüklensin)
    testSoldier->setType(SoldierTypes::Barbarian);

    // 3. Görsellik: Texture yoksa bile görelim diye KIRMIZI yapýyoruz
    testSoldier->getModel().setFillColor(sf::Color::Red);

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

            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

            // --- UI KORUMASI ---
            // Eðer mouse ekranýn üstündeki 50 piksellik alandaysa
            // VE týklama olayý gerçekleþtiyse, aþaðýdaki harita kodlarýný çalýþtýrma.
            const int UI_HEIGHT_LIMIT = 50;

            bool isMouseOnUI = (pixelPos.y < UI_HEIGHT_LIMIT);

            // Eðer bir þeye týklýyorsak (Sol veya Sað) ve UI üzerindeysek -> Yoksay
            if ((event.type == sf::Event::MouseButtonPressed) && isMouseOnUI) {
                continue; // Döngünün baþýna dön (Aþaðýdaki kodlarý okuma)
            }

            // --- HARÝTA KONTROLLERÝ ---
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // --- SOL TIK: SEÇÝM (SELECT) ---
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // UI'ya týklamadýysak (Basit kontrol: Mouse üst barýn altýnda mý?)
                if (pixelPos.y > 40) {
                    localPlayer.selectUnit(window);
                }
            }

            // --- SAÐ TIK: HAREKET EMRÝ ---
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {

                    // Mouse'un olduðu kareyi (Grid) bul
                    int targetGridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
                    int targetGridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

                    // Harita sýnýrlarý içinde mi?
                    if (targetGridX >= 0 && targetGridX < mapManager.getWidth() &&
                        targetGridY >= 0 && targetGridY < mapManager.getHeight()) {

                        Point baseTarget = { targetGridX, targetGridY };
                        std::set<Point> reservedTiles; // Dolu koltuklar
                        const auto& levelData = mapManager.getLevelData();

                        // 1. Hareket etmeyen diðer askerleri "Dolu Yer" say
                        for (const auto& entity : localPlayer.getEntities()) {
                            if (auto u = std::dynamic_pointer_cast<Unit>(entity)) {
                                if (!u->isSelected) { // Seçili deðilse yer kaplýyordur
                                    reservedTiles.insert(u->getGridPoint());
                                }
                            }
                        }

                        // 2. Seçili askerlere hedef daðýt
                        for (auto& entity : localPlayer.selected_entities) {
                            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                                // Formasyon için en yakýn boþ kareyi bul
                                Point specificGridTarget = PathFinder::findClosestFreeTile(
                                    baseTarget,
                                    levelData,
                                    mapManager.getWidth(),
                                    mapManager.getHeight(),
                                    reservedTiles
                                );

                                reservedTiles.insert(specificGridTarget); // Orayý kaptýk

                                // Grid -> Pixel dönüþümü (Merkeze gitmesi için)
                                float pixelX = specificGridTarget.x * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                                float pixelY = specificGridTarget.y * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;

                                // --- YENÝ EMRÝ VER (HATA ÇÖZÜMÜ BURADA) ---
                                // Eski: unit->path = ...
                                // Yeni: 
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

        // Harita verisini al (Çarpýþma kontrolü için)
        const auto& levelData = mapManager.getLevelData();
        int mapW = mapManager.getWidth();
        int mapH = mapManager.getHeight();

        // --- ASKERLERÝ GÜNCELLE ---
        for (auto& entity : localPlayer.getEntities()) {
            // Entity'i Unit'e dönüþtür
            if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                // ESKÝ KOD (HATA VEREN): 
                // if (!unit->path.empty()) ...

                // YENÝ KOD (DOÐRU):
                // Unit'in kendi update fonksiyonunu çaðýrýyoruz.
                // Artýk yolu deðil, duvarlarý kontrol ederek ilerliyor.
                unit->update(dt, levelData, mapW, mapH);
            }
        }

        // HUD Güncelle
        std::vector<int> res = localPlayer.getResources();
        hud.resourceBar.updateResources(res[0], res[3], res[1], res[2]);
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