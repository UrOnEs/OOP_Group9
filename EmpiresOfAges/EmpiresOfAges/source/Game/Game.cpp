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
    // --- DEÐÝÞÝKLÝK 1: TAM EKRAN BAÞLATMA ---
    // Masaüstü çözünürlüðünü alýyoruz (Örn: 1920x1080)
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();

    // Style::Fullscreen parametresi ekleyerek pencereyi oluþturuyoruz
    window.create(desktopMode, "Empires of Ages - RTS", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    // Kamerayý (View) ekran çözünürlüðüne göre ayarla ki görüntü bozulmasýn
    camera.setSize(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height));
    camera.setCenter(desktopMode.width / 2.0f, desktopMode.height / 2.0f);

    initUI();
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

    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));
    ghostGridRect.setSize(sf::Vector2f(32, 32));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    // Seçim Kutusu Görsel Ayarý
    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);
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

    int houseCost = GameRules::getBuildingCost(BuildTypes::House).wood;
    buildHouse.costText = "Cost: " + std::to_string(houseCost) + " Wood";

    buildHouse.description = "Population space for 5 units.";
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

        // UI Olaylarýný Ýþle
        uiManager.handleEvent(event);
        hud.handleEvent(event);

        // Pencere Kapatma (Alt+F4 vs. için)
        if (event.type == sf::Event::Closed)
            window.close();

        if (stateManager.getState() == GameState::Playing) {

            // 1. KLAVYE KISAYOLLARI
            if (event.type == sf::Event::KeyPressed) {
                // 'H' Tuþu: Ev Ýnþa Etme Modu
                if (event.key.code == sf::Keyboard::H) {
                    enterBuildMode(BuildTypes::House, "assets/icons/house_icon.jpg");
                }

                // --- DEÐÝÞÝKLÝK 2: ESC TUÞU ÝÞLEVÝ ---
                if (event.key.code == sf::Keyboard::Escape) {
                    if (isInBuildMode) {
                        // Eðer inþaat modundaysak önce inþaatý iptal et
                        cancelBuildMode();
                    }
                    else {
                        // Ýnþaat modunda deðilsek oyunu kapat (Tam ekrandan çýkýþ)
                        window.close();
                    }
                }
            }

            // Mouse Pozisyonlarýný Al
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // UI Korumasý: Mouse alt panelin veya üst barýn üzerindeyse haritaya týklama
            bool isMouseOnPanel = (pixelPos.y > 600);
            bool isMouseOnTopBar = (pixelPos.y < 50);
            bool isMouseOnUI = (isMouseOnPanel || isMouseOnTopBar);

            // Grid Koordinatlarýný Hesapla
            int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

            // 2. MOUSE TIKLAMA (BASMA) ÝÞLEMLERÝ
            if (event.type == sf::Event::MouseButtonPressed) {

                // --- SOL TIK ---
                if (event.mouseButton.button == sf::Mouse::Left) {

                    // Eðer UI üzerindeysek oyuna müdahale etme
                    if (isMouseOnUI) continue;

                    if (isInBuildMode) {
                        // --- ÝNÞAAT MODU ---

                        // 1. Maliyeti Öðren
                        GameRules::Cost cost = GameRules::getBuildingCost(pendingBuildingType);

                        // 2. Kaynak Kontrolü (Þimdilik sadece Odun üzerinden örnekliyoruz)
                        // Player.h içindeki getResources() sýrasý: 0:Wood, 1:Gold, 2:Stone, 3:Food
                        if (localPlayer.getResources()[0] >= cost.wood) {

                            // 3. Binayý Yerleþtirmeyi Dene (MapManager kontrol eder)
                            if (mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType)) {

                                // 4. Baþarýlýysa Kaynaðý Düþ
                                localPlayer.addWood(-cost.wood);
                                localPlayer.addGold(-cost.gold); // Varsa diðerlerini de düþ
                                localPlayer.addFood(-cost.food);
                                localPlayer.addStone(-cost.stone);

                                std::cout << "[GAME] Bina insa edildi!\n";

                                // Shift'e basýlý deðilse inþaat modundan çýk
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
                            cancelBuildMode();
                        }
                    }
                    else {
                        // --- SEÇÝM BAÞLANGICI (Normal Mod) ---
                        isSelecting = true;
                        selectionStartPos = worldPos;             // Kutunun baþladýðý yer
                        selectionBox.setSize(sf::Vector2f(0, 0)); // Kutuyu sýfýrla
                    }
                }

                // --- SAÐ TIK ---
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    if (isInBuildMode) {
                        // Ýnþaat modundaysak iptal et
                        cancelBuildMode();
                    }
                    else {
                        // --- HAREKET EMRÝ (PATHFINDING) ---

                        // Harita sýnýrlarý içinde miyiz?
                        if (gridX >= 0 && gridX < mapManager.getWidth() &&
                            gridY >= 0 && gridY < mapManager.getHeight()) {

                            Point baseTarget = { gridX, gridY };
                            std::set<Point> reservedTiles;
                            const auto& levelData = mapManager.getLevelData();

                            // A. Diðer askerlerin olduðu yerleri "Dolu" say
                            for (const auto& entity : localPlayer.getEntities()) {
                                if (auto u = std::dynamic_pointer_cast<Unit>(entity)) {
                                    if (!u->isSelected) {
                                        reservedTiles.insert(u->getGridPoint());
                                    }
                                }
                            }

                            // B. Seçili askerlere hedef daðýt ve yol çiz
                            for (auto& entity : localPlayer.selected_entities) {
                                if (auto unit = std::dynamic_pointer_cast<Unit>(entity)) {

                                    // Formasyon için en yakýn boþ kareyi bul
                                    Point specificGridTarget = PathFinder::findClosestFreeTile(
                                        baseTarget, levelData, mapManager.getWidth(), mapManager.getHeight(), reservedTiles
                                    );
                                    reservedTiles.insert(specificGridTarget); // Orayý rezerve et

                                    // A* ile yolu hesapla
                                    std::vector<Point> gridPath = PathFinder::findPath(
                                        unit->getGridPoint(), specificGridTarget, levelData, mapManager.getWidth(), mapManager.getHeight()
                                    );

                                    // Grid Koordinatlarýný Dünya (Pixel) Koordinatlarýna Çevir
                                    std::vector<sf::Vector2f> worldPath;
                                    for (const auto& p : gridPath) {
                                        float px = p.x * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                                        float py = p.y * mapManager.getTileSize() + mapManager.getTileSize() / 2.0f;
                                        worldPath.push_back(sf::Vector2f(px, py));
                                    }

                                    // Askere yolu ver
                                    unit->setPath(worldPath);
                                }
                            }
                        }
                    }
                }
            }

            // 3. MOUSE BIRAKMA (SEÇÝM BÝTÝÞÝ)
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (isSelecting) {
                    isSelecting = false;
                    bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

                    // Eðer kutu çok küçükse (sadece týklama yaptýysa)
                    if (std::abs(selectionBox.getSize().x) < 5.0f && std::abs(selectionBox.getSize().y) < 5.0f) {
                        localPlayer.selectUnit(window, camera, shift);
                    }
                    else {
                        // Alan seçimi (Sürükle býrak)
                        sf::FloatRect rect = selectionBox.getGlobalBounds();
                        localPlayer.selectUnitsInRect(rect, shift);
                    }

                    // Görseli temizle
                    selectionBox.setSize(sf::Vector2f(0, 0));
                }
            }

            // 4. MOUSE HAREKETÝ (SEÇÝM KUTUSU GÜNCELLEME)
            if (event.type == sf::Event::MouseMoved) {
                if (isSelecting) {
                    sf::Vector2i currentPixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f currentWorldPos = window.mapPixelToCoords(currentPixelPos, camera);

                    // Baþlangýç noktasýndan þu anki noktaya boyut hesapla
                    sf::Vector2f size = currentWorldPos - selectionStartPos;

                    selectionBox.setPosition(selectionStartPos);
                    selectionBox.setSize(size);
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

    // 1. Oyun Dünyasý Görünümü
    window.setView(camera);

    if (stateManager.getState() == GameState::Playing) {
        mapManager.draw(window);
        localPlayer.renderEntities(window);

        // --- SEÇÝM KUTUSUNU ÇÝZ ---
        if (isSelecting) {
            window.draw(selectionBox);
        }

        // --- ÝNÞAAT MODU (DÜZELTÝLMÝÞ KISIM) ---
        if (isInBuildMode) {
            // 1. Mouse'un haritadaki yerini bul
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

            // 2. Grid'e Yapýþtýr (Snap to Grid)
            // Koordinatý TileSize'a böl, tam sayýya yuvarla, tekrar çarp.
            int gx = static_cast<int>(worldPos.x / mapManager.getTileSize());
            int gy = static_cast<int>(worldPos.y / mapManager.getTileSize());

            float snapX = gx * mapManager.getTileSize();
            float snapY = gy * mapManager.getTileSize();

            // 3. Pozisyonu Güncelle (Mouse'u takip etmesi için BURASI ÞART)
            ghostBuildingSprite.setPosition(snapX, snapY);
            ghostGridRect.setPosition(snapX, snapY);

            // 4. Geçerlilik Kontrolü (Renk Deðiþimi)
            // Sadece harita sýnýrlarýný kontrol ediyoruz.
            // (Ýstersen buraya 'o karede bina var mý' kontrolü de eklenir)
            bool isValid = (gx >= 0 && gx < mapManager.getWidth() &&
                gy >= 0 && gy < mapManager.getHeight());

            if (isValid) {
                ghostBuildingSprite.setColor(sf::Color(0, 255, 0, 150)); // Yeþil (Müsait)
                ghostGridRect.setOutlineColor(sf::Color::Green);
            }
            else {
                ghostBuildingSprite.setColor(sf::Color(255, 0, 0, 150)); // Kýrmýzý (Yasak)
                ghostGridRect.setOutlineColor(sf::Color::Red);
            }

            // 5. Çiz
            window.draw(ghostBuildingSprite);
            window.draw(ghostGridRect);
        }
    }

    // 2. UI Görünümü
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

    sf::Texture& tex = AssetManager::getTexture(textureName);
    ghostBuildingSprite.setTexture(tex);

    // --- BOYUT HESAPLAMA ---
    float widthInTiles = 2.0f;
    float heightInTiles = 2.0f;

    if (type == BuildTypes::House) {
        widthInTiles = 1.0f;
        heightInTiles = 1.0f;
    }

    // Hedef piksel boyutu (Örn: 1x1 ise 64px, 2x2 ise 128px)
    float targetWidth = widthInTiles * mapManager.getTileSize();
    float targetHeight = heightInTiles * mapManager.getTileSize();

    // Resmi bu boyuta sýðdýr
    sf::Vector2u texSize = tex.getSize();
    ghostBuildingSprite.setScale(targetWidth / texSize.x, targetHeight / texSize.y);

    // Izgara karesini de (Outline) bu boyuta ayarla
    ghostGridRect.setSize(sf::Vector2f(targetWidth, targetHeight));

    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));
    std::cout << "[GAME] Insaat modu aktif.\n";
}

void Game::cancelBuildMode() {
    isInBuildMode = false;
    std::cout << "[GAME] Insaat modu iptal.\n";
}