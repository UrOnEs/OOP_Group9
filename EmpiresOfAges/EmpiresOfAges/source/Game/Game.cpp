#include "Game/Game.h"
#include "Systems/MovementSystem.h"
#include "Systems/ProductionSystem.h"
#include "Systems/BuildSystem.h"
#include "Systems/CombatSystem.h"
#include "Systems/ResourceSystem.h"
#include "Systems/SoundManager.h"

#include "Entity System/Entity Type/Unit.h"
#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/ResourceGenerator.h"
#include "Entity System/Entity Type/Barracks.h"
#include "Entity System/Entity Type/TownCenter.h"

#include <thread>             
#include <chrono>             
#include "Network/NetServer.h" 
#include "Network/NetClient.h" 

#include "Map/FogOfWar.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include "Map/PathFinder.h"
#include "Map/Point.h"
#include <set> 

#include "UI/AssetManager.h"
#include "Game/GameRules.h"

Game::Game(bool isHost, std::string serverIp, int playerIndex, int totalPlayerCount)
    : mapManager(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize),
    m_isHost(isHost),
    m_serverIp(serverIp),
    m_playerIndex(playerIndex),
    m_totalPlayerCount(totalPlayerCount),
    m_connectedClientCount(0)
{
    // 1. Window and View Settings
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    window.create(desktopMode, "Empire of Ages", sf::Style::Fullscreen);
    window.setFramerateLimit(60);

    camera.setSize(static_cast<float>(desktopMode.width), static_cast<float>(desktopMode.height));
    camera.setCenter(desktopMode.width / 2.0f, desktopMode.height / 2.0f);

    // 2. Initialize UI and Network
    initUI();
    initNetwork();

    // 3. Fog of War
    m_fogOfWar = std::make_unique<FogOfWar>(GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize);

    // 4. Construction and Selection Visuals
    ghostBuildingSprite.setColor(sf::Color(255, 255, 255, 150));

    ghostGridRect.setSize(sf::Vector2f(GameRules::TileSize, GameRules::TileSize));
    ghostGridRect.setFillColor(sf::Color::Transparent);
    ghostGridRect.setOutlineThickness(1);
    ghostGridRect.setOutlineColor(sf::Color::White);

    selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    selectionBox.setOutlineThickness(1.0f);
    selectionBox.setOutlineColor(sf::Color::Green);

    // 5. Background Music
    SoundManager::playMusic("assets/sounds/background_music.ogg");
}

void Game::startMatch(unsigned int seed) {
    std::cout << "[GAME] Starting Match. Seed: " << seed << " | My Index: " << m_playerIndex << std::endl;

    // A. Initialize Map with Seed
    mapManager.initialize(seed);

    // --- Initialize Minimap and HUD ---
    sf::Vector2u winSize = window.getSize();
    hud.init(winSize.x, winSize.y,
        GameRules::MapWidth, GameRules::MapHeight, GameRules::TileSize,
        mapManager.getLevelData());

    // B. Clear Previous Entities
    localPlayer.entities.clear();
    localPlayer.selected_entities.clear();
    enemyPlayer.entities.clear();
    enemyPlayer.selected_entities.clear();

    // C. Place Starting Buildings based on Spawn Points
    for (int i = 0; i < 4; ++i) {
        sf::Vector2i spawnGrid = GameRules::SpawnPoints[i];

        // Clear area for Town Center
        mapManager.clearArea(spawnGrid.x - 3, spawnGrid.y - 3, 10, 10);

        // If this index is MINE -> Add to LocalPlayer
        if (i == m_playerIndex) {
            localPlayer.setTeamColor((TeamColors)i);

            // 1. Town Center
            std::shared_ptr<Building> myTC = mapManager.tryPlaceBuilding(spawnGrid.x, spawnGrid.y, BuildTypes::TownCenter);
            if (myTC) {
                myTC->setTeam((TeamColors)i);
                localPlayer.addEntity(myTC);

                // 2. Villager
                std::shared_ptr<Villager> myVil = std::make_shared<Villager>();
                sf::Vector2f spawnPos = myTC->getPosition();
                spawnPos.y += 120.0f;
                myVil->setPosition(spawnPos);
                myVil->setTeam((TeamColors)i);
                localPlayer.addEntity(myVil);

                camera.setCenter(myTC->getPosition());
            }
        }
        // If this index is SOMEONE ELSE -> Add to EnemyPlayer
        else {
            // 1. Enemy Town Center
            std::shared_ptr<Building> enemyTC = mapManager.tryPlaceBuilding(spawnGrid.x, spawnGrid.y, BuildTypes::TownCenter);
            if (enemyTC) {
                enemyTC->setTeam((TeamColors)i);
                enemyPlayer.addEntity(enemyTC);

                // 2. Enemy Villager
                std::shared_ptr<Villager> enemyVil = std::make_shared<Villager>();
                sf::Vector2f spawnPos = enemyTC->getPosition();
                spawnPos.y += 120.0f;
                enemyVil->setPosition(spawnPos);
                enemyVil->setTeam((TeamColors)i);
                enemyPlayer.addEntity(enemyVil);
            }
        }
    }

    stateManager.setState(GameState::Playing);
}

void Game::initNetwork() {
    networkManager.setLogger([](const std::string& msg) {});

    // Packet Handler
    auto packetHandler = [this](uint64_t id, sf::Packet& pkt) {
        if (stateManager.getState() == GameState::Playing) {
            sf::Packet copyPkt = pkt;
            sf::Uint8 cmdRaw;

            if (!(copyPkt >> cmdRaw)) return;
            NetCommand cmd = (NetCommand)cmdRaw;

            // --- TRAIN UNIT ---
            if (cmd == NetCommand::TrainUnit) {
                int gx, gy, uType;
                if (copyPkt >> gx >> gy >> uType) {

                    if (m_isHost && id != 0) {
                        sf::Packet forwardPacket;
                        forwardPacket << (sf::Uint8)NetCommand::TrainUnit << gx << gy << uType;
                        networkManager.server()->sendToAllExcept(id, forwardPacket);
                    }

                    auto building = mapManager.getBuildingAt(gx, gy);

                    if (building && building->getTeam() != localPlayer.getTeamColor()) {
                        // Grant temporary resources to enemy to ensure production
                        enemyPlayer.addUnitLimit(50);
                        enemyPlayer.addFood(2000);
                        enemyPlayer.addWood(2000);
                        enemyPlayer.addGold(2000);
                        enemyPlayer.addStone(2000);

                        if (uType == 1 && std::dynamic_pointer_cast<TownCenter>(building)) {
                            ProductionSystem::startVillagerProduction(enemyPlayer, *std::dynamic_pointer_cast<TownCenter>(building));
                            std::cout << "[NET] Opponent (ID: " << id << ") is training a villager.\n";
                        }
                        else if (std::dynamic_pointer_cast<Barracks>(building)) {
                            auto bar = std::dynamic_pointer_cast<Barracks>(building);
                            SoldierTypes type = SoldierTypes::Barbarian;
                            if (uType == 3) type = SoldierTypes::Archer;
                            if (uType == 4) type = SoldierTypes::Wizard;
                            ProductionSystem::startProduction(enemyPlayer, *bar, type);
                            std::cout << "[NET] Opponent (ID: " << id << ") is training a soldier.\n";
                        }
                    }
                }
            }
            // --- PLACE BUILDING ---
            else if (cmd == NetCommand::PlaceBuilding) {
                int gx, gy, bTypeInt;
                int teamColorInt;

                if (copyPkt >> gx >> gy >> bTypeInt >> teamColorInt) {

                    if (m_isHost && id != 0) {
                        sf::Packet forwardPacket;
                        forwardPacket << (sf::Uint8)NetCommand::PlaceBuilding << gx << gy << bTypeInt << teamColorInt;
                        networkManager.server()->sendToAllExcept(id, forwardPacket);
                    }

                    BuildTypes type = (BuildTypes)bTypeInt;

                    if (mapManager.getBuildingAt(gx, gy) == nullptr) {
                        std::shared_ptr<Building> enemyBuilding = mapManager.tryPlaceBuilding(gx, gy, type);
                        if (enemyBuilding) {
                            enemyBuilding->setTeam((TeamColors)teamColorInt);
                            enemyBuilding->isConstructed = false;
                            enemyBuilding->health = 1.0f;
                            enemyPlayer.addEntity(enemyBuilding);
                            std::cout << "[NET] Opponent placed building (Color: " << teamColorInt << ") at: " << gx << "," << gy << "\n";
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
            lobbyManager = std::make_unique<LobbyManager>(&networkManager, true);
            networkManager.server()->setOnPacket(packetHandler);
            lobbyManager->start(1, "HostPlayer");
            lobbyManager->toggleReady(true);

            // Wait Logic
            if (m_totalPlayerCount == 1) {
                std::cout << "[GAME] Single player game. Starting immediately.\n";
                m_startGameTimer = 0.5f;
            }
            else {
                std::cout << "[GAME] Waiting for " << m_totalPlayerCount << " players...\n";
                networkManager.server()->setOnClientConnected([this](uint64_t clientId) {
                    m_connectedClientCount++;
                    std::cout << "[GAME] Player (" << clientId << ") connected. Status: "
                        << m_connectedClientCount << "/" << (m_totalPlayerCount - 1) << "\n";

                    if (m_connectedClientCount >= (m_totalPlayerCount - 1)) {
                        std::cout << "[GAME] ALL READY! Starting match...\n";
                        m_startGameTimer = 0.1f;
                    }
                    });
            }
        }
        else {
            std::cerr << "[GAME] ERROR: Could not start server!" << std::endl;
        }
    }
    else {
        // --- CLIENT ---
        std::cout << "[GAME] Connecting to server: " << m_serverIp << "...\n";
        if (networkManager.startClient(m_serverIp, 54000)) {
            lobbyManager = std::make_unique<LobbyManager>(&networkManager, false);
            networkManager.client()->setOnPacket([packetHandler](sf::Packet& pkt) { packetHandler(0, pkt); });
            lobbyManager->start(0, "ClientPlayer");
            lobbyManager->toggleReady(true);

            sf::Packet dummy; dummy << (sf::Uint8)NetCommand::None;
            networkManager.client()->sendReliable(dummy);
            std::cout << "[GAME] Client connected. Waiting for start signal...\n";
        }
        else {
            std::cerr << "[GAME] ERROR: Could not connect!\n";
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

    hud.settingsPanel.setOnQuitGame([this]() {
        std::cout << "[GAME] Quitting game...\n";
        this->window.close();
        });
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
//                                  MAIN EVENT LOOP
// ======================================================================================

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {

        uiManager.handleEvent(event);

        if (event.type == sf::Event::Closed) {
            window.close();
        }

        // Only process HUD and Game Inputs if Playing
        if (stateManager.getState() == GameState::Playing) {

            hud.handleEvent(event);

            // 1. KEYBOARD INPUTS
            if (event.type == sf::Event::KeyPressed) {
                handleKeyboardInput(event);
            }

            // 2. MOUSE INPUTS
            if (event.type == sf::Event::MouseButtonPressed ||
                event.type == sf::Event::MouseButtonReleased ||
                event.type == sf::Event::MouseMoved) {
                handleMouseInput(event);
            }
        }
    }
}

// ======================================================================================
//                                  KEYBOARD CONTROLS
// ======================================================================================

void Game::handleKeyboardInput(const sf::Event& event) {
    // Shortcuts
    if (event.key.code == sf::Keyboard::H) enterBuildMode(BuildTypes::House, "assets/buildings/house.png");
    if (event.key.code == sf::Keyboard::B) enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png");
    if (event.key.code == sf::Keyboard::C) enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png");
    if (event.key.code == sf::Keyboard::M) enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png");

    // Cancel / Exit
    if (event.key.code == sf::Keyboard::Escape) {
        if (isInBuildMode) cancelBuildMode();
        else window.close();
    }

    // T: TRAIN
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

    // D: DESTROY
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
                std::cout << "[GAME] Building destroyed.\n";
            }
        }
    }

    // K: Damage Test (Debug)
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

            isDevMode = !isDevMode;
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
//                                  MOUSE CONTROLS
// ======================================================================================

void Game::handleMouseInput(const sf::Event& event) {
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, camera);

    // --- MINIMAP CONTROL ---
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f newCamPos;
        if (hud.minimap.handleClick(pixelPos, newCamPos)) {
            camera.setCenter(newCamPos);
            return;
        }
    }

    // Block interaction if mouse is over UI
    if (event.type == sf::Event::MouseButtonPressed && hud.isMouseOverUI(pixelPos)) return;

    // --- LEFT CLICK ---
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        onLeftClick(worldPos, pixelPos);
    }

    // --- RIGHT CLICK ---
    else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
        onRightClick(worldPos);
    }

    // --- SELECTION BOX UPDATE ---
    else if (event.type == sf::Event::MouseMoved && isSelecting) {
        selectionBox.setSize(worldPos - selectionStartPos);
    }

    // --- SELECTION END ---
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isSelecting) {
            isSelecting = false;
            bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

            // Point or Box selection?
            if (std::abs(selectionBox.getSize().x) < 5.0f && std::abs(selectionBox.getSize().y) < 5.0f) {
                localPlayer.selectUnit(window, camera, shift);
            }
            else {
                localPlayer.selectUnitsInRect(selectionBox.getGlobalBounds(), shift);
            }
            selectionBox.setSize(sf::Vector2f(0, 0));

            // Update HUD
            if (!localPlayer.selected_entities.empty()) {
                hud.selectedPanel.setVisible(true);
                auto entity = localPlayer.selected_entities[0];

                auto handleProductionResult = [this](ProductionResult result) {
                    switch (result) {
                    case ProductionResult::Success: break;
                    case ProductionResult::PopulationFull:
                        showWarning("Population Limit Reached! Build Houses.");
                        break;
                    case ProductionResult::InsufficientFood:
                        showWarning("Insufficient Resources: Food required.");
                        break;
                    case ProductionResult::InsufficientWood:
                        showWarning("Insufficient Resources: Wood required.");
                        break;
                    case ProductionResult::InsufficientGold:
                        showWarning("Insufficient Resources: Gold required.");
                        break;
                    case ProductionResult::QueueFull:
                        showWarning("Production Queue Full!");
                        break;
                    default:
                        break;
                    }
                    };

                // Setup Button Callbacks
                std::vector<Ability> uiAbilities = entity->getAbilities();
                for (auto& ab : uiAbilities) {
                    if (ab.getId() == 1) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::House, "assets/buildings/house.png"); });
                    else if (ab.getId() == 2) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Barrack, "assets/buildings/barrack.png"); });
                    else if (ab.getId() == 3) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::Farm, "assets/buildings/mill.png"); });
                    else if (ab.getId() == 4) ab.setOnClick([this]() { this->enterBuildMode(BuildTypes::TownCenter, "assets/buildings/castle.png"); });

                    else if (ab.getId() == 10) { // Train Villager
                        if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity))
                            ab.setOnClick([this, tc, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startVillagerProduction(localPlayer, *tc);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(tc->getGridPoint().x, tc->getGridPoint().y, 1);
                            }
                                });
                    }
                    else if (ab.getId() == 11) { // Barbarian
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Barbarian);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(b->getGridPoint().x, b->getGridPoint().y, 2);
                            }
                                });
                    }
                    else if (ab.getId() == 12) { // Archer
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Archer);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(b->getGridPoint().x, b->getGridPoint().y, 3);
                            }
                                });
                    }
                    else if (ab.getId() == 13) { // Wizard
                        if (auto b = std::dynamic_pointer_cast<Barracks>(entity))
                            ab.setOnClick([this, b, handleProductionResult]() {
                            ProductionResult res = ProductionSystem::startProduction(localPlayer, *b, SoldierTypes::Wizard);
                            handleProductionResult(res);

                            if (res == ProductionResult::Success) {
                                this->sendTrainCommand(b->getGridPoint().x, b->getGridPoint().y, 4);
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
//                                  LEFT CLICK (BUILD / SELECT)
// ======================================================================================

void Game::onLeftClick(const sf::Vector2f& worldPos, const sf::Vector2i& pixelPos) {
    int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
    int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

    if (isInBuildMode) {
        // --- CONSTRUCTION ---
        GameRules::Cost cost = GameRules::getBuildingCost(pendingBuildingType);

        bool canAfford = localPlayer.getResources()[0] >= cost.wood &&
            localPlayer.getResources()[1] >= cost.gold &&
            localPlayer.getResources()[2] >= cost.stone &&
            localPlayer.getResources()[3] >= cost.food;

        if (canAfford) {
            std::shared_ptr<Building> placed = mapManager.tryPlaceBuilding(gridX, gridY, pendingBuildingType);
            if (placed) {
                placed->isConstructed = false;
                placed->health = 1.0f;

                placed->setTeam(localPlayer.getTeamColor());

                localPlayer.addEntity(placed);

                localPlayer.addWood(-cost.wood);
                localPlayer.addGold(-cost.gold);
                localPlayer.addStone(-cost.stone);
                localPlayer.addFood(-cost.food);

                sendBuildCommand(gridX, gridY, (int)pendingBuildingType);

                std::cout << "[GAME] Foundation laid! Waiting for construction.\n";

                // Auto-assign builder if a single villager is selected
                if (localPlayer.selected_entities.size() == 1) {
                    if (auto vil = std::dynamic_pointer_cast<Villager>(localPlayer.selected_entities[0])) {
                        vil->startBuilding(placed);
                    }
                }

                std::cout << "[GAME] Building constructed!\n";
                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) cancelBuildMode();
            }
            else {
                std::cout << "[GAME] Area blocked or invalid!\n";
            }
        }
        else {
            showWarning("Insufficient Resources!");
            cancelBuildMode();
        }
    }
    else {
        bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

        // 1. Try selecting own units
        localPlayer.selectUnit(window, camera, shift);

        // 2. If nothing selected (or empty click), check enemies
        if (localPlayer.selected_entities.empty()) {
            enemyPlayer.selectUnit(window, camera, false);

            if (!enemyPlayer.selected_entities.empty()) {
                auto entity = enemyPlayer.selected_entities[0];

                // Fog check for enemies
                if (m_fogOfWar && !m_fogOfWar->isVisible(entity->getPosition().x, entity->getPosition().y)) {
                    // Do not select unseen units (Buildings might be exceptions if explored)
                    if (!std::dynamic_pointer_cast<Building>(entity)) {
                        enemyPlayer.selected_entities.clear();
                        hud.selectedPanel.setVisible(false);
                        return;
                    }
                }

                std::vector<Ability> emptyAbilities;
                hud.selectedPanel.updateQueue({}, 0.0f);
                hud.selectedPanel.setVisible(true);
                hud.selectedPanel.updateSelection(
                    "[ENEMY] " + entity->getName(),
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
            enemyPlayer.selected_entities.clear();

            auto entity = localPlayer.selected_entities[0];
            hud.selectedPanel.setVisible(true);

            hud.selectedPanel.updateSelection(
                entity->getName(),
                (int)entity->health, entity->getMaxHealth(),
                entity->getIcon(),
                entity->getAbilities()
            );
        }

        // 3. Start selection box
        isSelecting = true;
        selectionStartPos = worldPos;
        selectionBox.setPosition(selectionStartPos);
        selectionBox.setSize(sf::Vector2f(0, 0));
    }
}

// ======================================================================================
//                                  RIGHT CLICK (MOVE / ATTACK / HARVEST)
// ======================================================================================

void Game::onRightClick(const sf::Vector2f& worldPos) {
    if (isInBuildMode) {
        cancelBuildMode();
        return;
    }

    int gridX = static_cast<int>(worldPos.x / mapManager.getTileSize());
    int gridY = static_cast<int>(worldPos.y / mapManager.getTileSize());

    std::shared_ptr<Building> clickedBuilding = mapManager.getBuildingAt(gridX, gridY);
    auto resGen = std::dynamic_pointer_cast<ResourceGenerator>(clickedBuilding);

    std::shared_ptr<Entity> clickedEnemyUnit = nullptr;
    for (auto& ent : enemyPlayer.getEntities()) {

        if (!ent->getIsAlive()) continue;

        sf::Vector2f diff = ent->getPosition() - worldPos;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        if (distSq < 20.0f * 20.0f) {
            clickedEnemyUnit = ent;
            break;
        }
    }

    // --- FOG OF WAR CHECK ---
    if (clickedEnemyUnit && m_fogOfWar) {
        if (!m_fogOfWar->isVisible(clickedEnemyUnit->getPosition().x, clickedEnemyUnit->getPosition().y)) {
            clickedEnemyUnit = nullptr;
        }
    }

    if (clickedBuilding) {
        // --- A. BUILDING CLICKED ---

        // 1. Harvest Resource
        if (resGen) {
            bool sentVillager = false;
            for (auto& entity : localPlayer.selected_entities) {
                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                    villager->startHarvesting(resGen);
                    sentVillager = true;
                }
            }
            if (sentVillager) std::cout << "[GAME] Villagers sent to harvest.\n";
        }

        // 2. Construction/Repair Order
        if (clickedBuilding->getTeam() == localPlayer.getTeamColor() && !clickedBuilding->isConstructed) {
            for (auto& entity : localPlayer.selected_entities) {
                if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                    villager->startBuilding(clickedBuilding);
                }
            }
            std::cout << "[GAME] Villagers sent to build.\n";
            return;
        }

        // 3. Attack Building
        bool sentSoldier = false;
        for (auto& entity : localPlayer.selected_entities) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->setTarget(clickedBuilding);
                sentSoldier = true;
            }
        }
        if (sentSoldier) std::cout << "[GAME] Attack order on building!\n";

    }
    else if (clickedEnemyUnit) {
        // --- B. ATTACK ENEMY UNIT ---
        std::cout << "[GAME] Attack order on enemy unit!\n";
        for (auto& entity : localPlayer.selected_entities) {
            if (auto soldier = std::dynamic_pointer_cast<Soldier>(entity)) {
                soldier->setTarget(clickedEnemyUnit);
            }
        }
    }
    else {
        // --- C. GROUND CLICK (MOVE) ---

        // 1. Stop Villagers
        for (auto& entity : localPlayer.selected_entities) {
            if (auto villager = std::dynamic_pointer_cast<Villager>(entity)) {
                villager->stopHarvesting();
            }
        }

        // 2. Move Soldiers
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

    if (m_isHost && m_startGameTimer > 0.0f) {
        m_startGameTimer -= dt;
        if (m_startGameTimer <= 0.0f) {
            if (lobbyManager) {
                std::cout << "[GAME] Timer finished, sending start signal...\n";
                lobbyManager->startGame();
            }
            m_startGameTimer = -1.0f;
        }
    }

    if (stateManager.getState() == GameState::Playing) {

        if (m_fogOfWar) {
            m_fogOfWar->update(localPlayer.getEntities());
        }

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

        // --- UPDATE ENTITIES ---
        for (auto& entity : localPlayer.getEntities()) {
            if (auto u = std::dynamic_pointer_cast<Unit>(entity)) u->update(dt, levelData, mapW, mapH);
            if (auto v = std::dynamic_pointer_cast<Villager>(entity)) v->updateVillager(dt, allBuildings, localPlayer, levelData, mapW, mapH);
            if (auto s = std::dynamic_pointer_cast<Soldier>(entity)) s->updateSoldier(dt, enemyPlayer.getEntities());
            if (auto b = std::dynamic_pointer_cast<Barracks>(entity)) ProductionSystem::update(localPlayer, *b, dt, mapManager);
            if (auto tc = std::dynamic_pointer_cast<TownCenter>(entity)) ProductionSystem::updateTC(localPlayer, *tc, dt, mapManager);
        }

        // --- UPDATE RESOURCES ---
        for (auto& building : mapManager.getBuildings()) {
            if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {
                if (res->isWorking()) ResourceSystem::update(localPlayer, *res, dt);
            }
        }

        // --- UPDATE SELECTION UI ---
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

        // --- UPDATE ENEMIES ---
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

    if (stateManager.getState() == GameState::Playing) {
        mapManager.draw(window);

        localPlayer.renderEntities(window);
        for (auto& entity : localPlayer.getEntities()) {
            if (entity->getIsAlive()) entity->renderEffects(window);
        }

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

        if (m_fogOfWar) m_fogOfWar->draw(window);

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

    // --- UI LAYER ---
    window.setView(window.getDefaultView());

    if (stateManager.getState() == GameState::Playing) {
        hud.draw(window);
        uiManager.draw(window);
        drawWarning(window);
    }
    else {
        // Waiting Screen
        sf::Font& font = AssetManager::getFont("assets/fonts/arial.ttf");

        sf::Text waitText;
        waitText.setFont(font);
        waitText.setCharacterSize(24);
        waitText.setFillColor(sf::Color::White);

        if (m_isHost) {
            waitText.setString("Waiting for Players... (" +
                std::to_string(m_connectedClientCount) + "/" +
                std::to_string(m_totalPlayerCount - 1) + ")");
        }
        else {
            waitText.setString("Waiting for Host to Start...");
        }

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

    std::cout << "[GAME] Build mode active (Size: " << widthInTiles << "x" << heightInTiles << ")\n";
}

void Game::showWarning(const std::string& message) {
    warningMsg = message;
    warningClock.restart();
    isWarningActive = true;
}

void Game::drawWarning(sf::RenderWindow& window) {
    if (!isWarningActive || warningClock.getElapsedTime().asSeconds() > 2.5f) {
        isWarningActive = false;
        return;
    }

    sf::Font& font = AssetManager::getFont("assets/fonts/arial.ttf");
    sf::Text warnText(warningMsg, font, 24);
    warnText.setFillColor(sf::Color::Red);
    warnText.setOutlineColor(sf::Color::Black);
    warnText.setOutlineThickness(1.5f);

    sf::FloatRect textRect = warnText.getLocalBounds();
    warnText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

    float screenX = window.getSize().x / 2.0f;
    float screenY = window.getSize().y - 200.0f;
    warnText.setPosition(screenX, screenY);

    sf::RectangleShape bg(sf::Vector2f(textRect.width + 20, textRect.height + 10));
    bg.setFillColor(sf::Color(0, 0, 0, 180));
    bg.setOrigin(bg.getSize().x / 2.0f, bg.getSize().y / 2.0f);
    bg.setPosition(screenX, screenY);

    window.draw(bg);
    window.draw(warnText);
}

void Game::cancelBuildMode() {
    isInBuildMode = false;
    std::cout << "[GAME] Build mode cancelled.\n";
}

void Game::sendTrainCommand(int gridX, int gridY, int unitTypeID) {
    sf::Packet packet;
    packet << (sf::Uint8)NetCommand::TrainUnit;
    packet << gridX << gridY;
    packet << unitTypeID;

    if (m_isHost) {
        networkManager.server()->sendToAllReliable(packet);
    }
    else {
        networkManager.client()->sendReliable(packet);
    }
}

void Game::sendBuildCommand(int gridX, int gridY, int buildTypeID) {
    sf::Packet packet;
    packet << (sf::Uint8)NetCommand::PlaceBuilding;
    packet << gridX << gridY;
    packet << buildTypeID;
    packet << (sf::Int32)localPlayer.getTeamColor();

    if (m_isHost) {
        networkManager.server()->sendToAllReliable(packet);
    }
    else {
        networkManager.client()->sendReliable(packet);
    }
}