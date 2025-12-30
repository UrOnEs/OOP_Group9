// main.cpp
#include "Game/Game.h"
#include <iostream>

// main.cpp - RTS Network: Hizalanmýþ Scroll Butonlar

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <functional>
#include <chrono> 
#include <thread>

// --- Kendi Header Dosyalarýnýz ---
#include "Network/NetworkManager.h"
#include "Network/LobbyManager.h"
#include "Network/NetServer.h"
#include "Network/NetClient.h"
#include "Network/LANDiscovery.h"
#include "UI/UIManager.h" 
#include "game/Game.h"    

// --- Global Sabitler ---
const unsigned short GAME_PORT = 54000;
const unsigned short DISCOVERY_PORT = 50000;
const std::string SERVER_NAME = "RTS Host Lobby";

// --- Global Deðiþkenler ---
bool g_isEnteringIP = false;
std::string g_ipInputString = "";
bool g_isEnteringName = true; // Baþlangýçta true olsun ki isim sorsun
std::string g_nameInputString = "";
std::string g_playerCurrentName = "Player";
std::string g_targetIP = "127.0.0.1";

// --- Global Yöneticiler ---
NetworkManager g_netManager;
LobbyManager* g_lobbyManager = nullptr;
bool g_isHost = false;
GameState g_currentState = GameState::Menu;

std::vector<LANDiscovery::ServerInfo> g_foundServers;

// --- Yardýmcý Fonksiyonlar ---

sf::Color getColorFromIndex(int index) {
    switch (index) {
    case 0: return sf::Color::Red;
    case 1: return sf::Color::Blue;
    case 2: return sf::Color::Green;
    case 3: return sf::Color(160, 32, 240);
    default: return sf::Color::White;
    }
}

bool isSelfReady() {
    if (!g_lobbyManager) return false;
    uint64_t myId = g_lobbyManager->selfId();
    for (const auto& p : g_lobbyManager->players()) {
        if (p.id == myId) return p.ready;
    }
    return false;
}

// 1. HOST BAÞLATMA
void startHost() {
    std::cout << "[UI] Host baslatiliyor..." << std::endl;
    if (!g_netManager.startServer(GAME_PORT)) return;

    g_netManager.discovery()->startServer(GAME_PORT, DISCOVERY_PORT, SERVER_NAME);
    g_lobbyManager = new LobbyManager(&g_netManager, true);
    g_lobbyManager->start(1, g_playerCurrentName);
    g_isHost = true;

    g_netManager.server()->setOnPacket([](uint64_t clientId, sf::Packet& pkt) {
        if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(clientId, pkt);
        });

    g_netManager.server()->setOnClientDisconnected([](uint64_t clientId) {
        if (g_lobbyManager) g_lobbyManager->removePlayer(clientId);
        });

    g_lobbyManager->setOnGameStart([]() { g_currentState = GameState::Playing; });
    g_currentState = GameState::LobbyRoom;
}

// 2. SUNUCUYA BAÐLANMA
void connectToServer(const LANDiscovery::ServerInfo& info) {
    g_targetIP = info.address.toString();
    g_netManager.discovery()->stop();

    if (g_netManager.startClient(info.address.toString(), info.port)) {
        g_lobbyManager = new LobbyManager(&g_netManager, false);

        // Ýsmi ayarlýysa onu kullan, deðilse rastgele yap
        if (g_playerCurrentName != "Player") {
            g_lobbyManager->start(0, g_playerCurrentName);
        }
        else {
            std::string randomName = "Oyuncu_" + std::to_string(rand() % 100);
            g_lobbyManager->start(0, randomName);
        }

        g_netManager.client()->setOnPacket([](sf::Packet& pkt) {
            if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(0, pkt);
            });

        g_netManager.client()->setOnDisconnected([]() {
            if (g_lobbyManager) { delete g_lobbyManager; g_lobbyManager = nullptr; }
            g_currentState = GameState::Menu;
            });

        g_lobbyManager->setOnGameStart([]() { g_currentState = GameState::Playing; });
        g_currentState = GameState::LobbyRoom;
    }
}

// Main.cpp içine eklenen direct connect
void connectToDirectIP(const std::string& ipStr) {
    g_targetIP = ipStr;
    g_netManager.discovery()->stop();

    if (g_netManager.startClient(ipStr, GAME_PORT)) {
        g_lobbyManager = new LobbyManager(&g_netManager, false);

        if (g_playerCurrentName != "Player") {
            g_lobbyManager->start(0, g_playerCurrentName);
        }
        else {
            std::string randomName = "Oyuncu_" + std::to_string(rand() % 100);
            g_lobbyManager->start(0, randomName);
        }

        g_netManager.client()->setOnPacket([](sf::Packet& pkt) {
            if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(0, pkt);
            });

        g_netManager.client()->setOnDisconnected([]() {
            if (g_lobbyManager) { delete g_lobbyManager; g_lobbyManager = nullptr; }
            g_currentState = GameState::Menu;
            });

        g_lobbyManager->setOnGameStart([]() { g_currentState = GameState::Playing; });
        g_currentState = GameState::LobbyRoom;
    }
    else {
        std::cerr << "Baglanti hatasi: IP adresi gecersiz veya sunucu kapali!" << std::endl;
    }
}

// 3. ARAMA MODU
void startDiscoveryMode() {
    g_foundServers.clear();
    g_netManager.discovery()->stop();
    g_netManager.discovery()->startClient(DISCOVERY_PORT);
    g_isHost = false;

    g_netManager.discovery()->setOnServerFound([](const LANDiscovery::ServerInfo& info) {
        bool exists = false;
        for (const auto& s : g_foundServers) {
            if (s.address == info.address && s.port == info.port) { exists = true; break; }
        }
        if (!exists) g_foundServers.push_back(info);
        });
}

// 4. AYRILMA
void leaveLobby() {
    g_netManager.discovery()->stop();
    if (!g_isHost && g_netManager.client() && g_netManager.client()->isConnected()) {
        sf::Packet pkt; pkt << static_cast<sf::Int32>(4); // LeaveLobby Command
        g_netManager.client()->send(pkt);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        g_netManager.client()->disconnect();
    }
    if (g_isHost) {
        if (g_lobbyManager) g_lobbyManager->closeLobby();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (g_netManager.server()) g_netManager.server()->stop();
    }
    if (g_lobbyManager) { delete g_lobbyManager; g_lobbyManager = nullptr; }
    g_isHost = false;
    g_foundServers.clear();
    g_currentState = GameState::Menu;
}

// --- MAIN ---
int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    g_netManager.setLogger([](const std::string& msg) { /* std::cout << msg << std::endl; */ });

    const int WIN_W = 900;
    const int WIN_H = 600;

    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "RTS Proje - Final UI", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // --- 1. Font Yükleme ---
    sf::Font font;
    if (!font.loadFromFile("assets/MenuUI/arial.ttf")) {
        std::cerr << "HATA: assets/arial.ttf yok." << std::endl;
    }
    sf::Font menuFont;
    if (!menuFont.loadFromFile("assets/MenuUI/SuperPixel.ttf")) {
        std::cerr << "HATA: SuperPixel.ttf bulunamadi" << std::endl;
    }

    // --- 2. Button Texture ---
    sf::Texture btnTexture;
    if (!btnTexture.loadFromFile("assets/MenuUI/button2.png")) {
        std::cerr << "HATA: assets/button2.png bulunamadi!" << std::endl;
    }

    // --- 3. BACKGROUND ---
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    sf::Texture lobbySelectionBgTexture;
    sf::Sprite lobbySelectionBgSprite;
    sf::Texture lobbyBgTexture;
    sf::Sprite lobbyBgSprite;
    bool hasBg = false;
    bool hasLobbySelectionBg = false;
    bool hasLobbyBg = false;

    if (bgTexture.loadFromFile("assets/MenuUI/background.png")) {
        hasBg = true;
        bgSprite.setTexture(bgTexture);
        sf::Vector2u texSize = bgTexture.getSize();
        bgSprite.setScale((float)WIN_W / texSize.x, (float)WIN_H / texSize.y);
    }
    else if (bgTexture.loadFromFile("assets/MenuUI/background.jpg")) {
        hasBg = true;
        bgSprite.setTexture(bgTexture);
        sf::Vector2u texSize = bgTexture.getSize();
        bgSprite.setScale((float)WIN_W / texSize.x, (float)WIN_H / texSize.y);
    }

    if (lobbySelectionBgTexture.loadFromFile("assets/MenuUI/lobbySelection.png")) {
        hasLobbySelectionBg = true;
        lobbySelectionBgSprite.setTexture(lobbySelectionBgTexture);
        sf::Vector2u texSize = lobbySelectionBgTexture.getSize();
        lobbySelectionBgSprite.setScale((float)WIN_W / texSize.x, (float)WIN_H / texSize.y);
    }

    if (lobbyBgTexture.loadFromFile("assets/MenuUI/lobbyBackground.jpg")) {
        hasLobbyBg = true;
        lobbyBgSprite.setTexture(lobbyBgTexture);
        sf::Vector2u texSize = lobbyBgTexture.getSize();
        lobbyBgSprite.setScale((float)WIN_W / texSize.x, (float)WIN_H / texSize.y);
    }

    // Oyun Ýsmi Logosu
    sf::Texture gameNameTexture;
    sf::Sprite gameNameSprite;
    if (gameNameTexture.loadFromFile("assets/MenuUI/Empire of Ages1.png")) {
        gameNameSprite.setTexture(gameNameTexture);
        gameNameSprite.setOrigin(gameNameSprite.getLocalBounds().width / 2.0f, gameNameSprite.getLocalBounds().height / 2.0f);
        gameNameSprite.setPosition(450.0f, 120.0f);
        gameNameSprite.setScale({ 0.25f, 0.25f });
    }

    // --- UI Butonlar ---
    UIPanel mainMenu({ 400, 300 }, { 250, 250 });

    UIButton btnStart;
    btnStart.setPosition(310, 300);
    btnStart.setSize(280, 65);
    btnStart.setTexture(btnTexture, 280, 65);
    btnStart.setText("Play", menuFont);
    btnStart.setCallback([&]() { g_currentState = GameState::LobbySelection; });

    UIButton btnExit;
    btnExit.setPosition(310, 380);
    btnExit.setSize(280, 65);
    btnExit.setTexture(btnTexture, 280, 65);
    btnExit.setText("Exit", menuFont);
    btnExit.setCallback([&]() { window.close(); });

    mainMenu.addButton(btnStart);
    mainMenu.addButton(btnExit);

    // Lobi Seçim Ekraný
    UIPanel selectionMenu({ 0, 0 }, { 50, 50 });

    UIButton btnCreate;
    btnCreate.setPosition(25, 120);
    btnCreate.setSize(280, 65);
    btnCreate.setTexture(btnTexture, 280, 65);
    btnCreate.setText("Lobi Kur (Host)", menuFont);
    btnCreate.setCallback(startHost);

    UIButton btnSearch;
    btnSearch.setPosition(25, 200);
    btnSearch.setSize(280, 65);
    btnSearch.setTexture(btnTexture, 280, 65);
    btnSearch.setText("Lobi Ara (Yenile)", menuFont);
    btnSearch.setCallback(startDiscoveryMode);

    UIButton btnBack;
    btnBack.setPosition(25, 280);
    btnBack.setSize(280, 65);
    btnBack.setTexture(btnTexture, 280, 65);
    btnBack.setText("Back", menuFont);
    btnBack.setCallback([&]() {
        g_netManager.discovery()->stop();
        g_currentState = GameState::Menu;
        });

    UIButton btnDirectConnect;
    btnDirectConnect.setPosition(25, 360);
    btnDirectConnect.setSize(280, 65);
    btnDirectConnect.setTexture(btnTexture, 280, 65);
    btnDirectConnect.setText("IP ile Baglan", menuFont);
    btnDirectConnect.setCallback([&]() {
        g_isEnteringIP = true;
        g_ipInputString = "";
        });

    selectionMenu.addButton(btnCreate);
    selectionMenu.addButton(btnSearch);
    selectionMenu.addButton(btnBack);
    selectionMenu.addButton(btnDirectConnect);

    // Lobi Odasý Butonlarý
    UIButton btnReady;
    btnReady.setPosition(350, 500);
    btnReady.setSize(200, 60);
    btnReady.setTexture(btnTexture, 200, 60);
    btnReady.setText("Ready", menuFont);
    btnReady.setCallback([]() { if (g_lobbyManager) g_lobbyManager->toggleReady(!isSelfReady()); });

    UIButton btnStartGame;
    btnStartGame.setPosition(600, 500);
    btnStartGame.setSize(200, 60);
    btnStartGame.setTexture(btnTexture, 200, 60);
    btnStartGame.setText("Start", menuFont);
    btnStartGame.setCallback([]() { if (g_lobbyManager && g_isHost) g_lobbyManager->startGame(); });

    UIButton btnLeave;
    btnLeave.setPosition(50, 500);
    btnLeave.setSize(120, 50);
    btnLeave.setTexture(btnTexture, 120, 50);
    btnLeave.setText("Leave", menuFont);
    btnLeave.setCallback(leaveLobby);


    // --- OYUN DÖNGÜSÜ ---
    sf::Clock dtClock;
    sf::Clock discoveryTimer;

    while (window.isOpen()) {
        float dt = dtClock.restart().asSeconds();
        g_netManager.update(dt);

        // Ping Atma
        if (!g_isHost && g_lobbyManager == nullptr && g_currentState == GameState::LobbySelection) {
            if (discoveryTimer.getElapsedTime().asSeconds() > 1.0f) {
                g_netManager.discovery()->sendDiscoveryRequest();
                discoveryTimer.restart();
            }
        }

        // Görsel Update
        if (g_currentState == GameState::LobbyRoom) {
            btnReady.setText(isSelfReady() ? "Readyn't" : "Ready", menuFont);
        }

        // Event
        sf::Event event;
        while (window.pollEvent(event)) {
            // NAME Input
            if (g_isEnteringName) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) { // Backspace
                        if (!g_nameInputString.empty()) g_nameInputString.pop_back();
                    }
                    else if (event.text.unicode == 13) { // Enter
                        if (!g_nameInputString.empty()) {
                            g_playerCurrentName = g_nameInputString;
                            g_isEnteringName = false;
                            std::cout << "[UI] Isim ayarlandi: " << g_playerCurrentName << std::endl;
                        }
                    }
                    else if (event.text.unicode < 128) {
                        if (g_nameInputString.length() < 12) {
                            g_nameInputString += static_cast<char>(event.text.unicode);
                        }
                    }
                }
            }
            // IP Input
            if (g_isEnteringIP) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) {
                        if (!g_ipInputString.empty()) g_ipInputString.pop_back();
                    }
                    else if (event.text.unicode == 13) {
                        if (!g_ipInputString.empty()) {
                            connectToDirectIP(g_ipInputString);
                            g_isEnteringIP = false;
                        }
                    }
                    else if (event.text.unicode < 128) {
                        g_ipInputString += static_cast<char>(event.text.unicode);
                    }
                }
                if (event.type == sf::Event::MouseButtonPressed) g_isEnteringIP = false;
            }

            if (event.type == sf::Event::Closed) {
                leaveLobby();
                window.close();
            }

            if (g_currentState == GameState::Menu && !g_isEnteringName) mainMenu.handleEvent(event);
            else if (g_currentState == GameState::LobbySelection) {
                selectionMenu.handleEvent(event);

                if (!g_isHost && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;

                    int y = 150; // Listedeki y ile ayný olmalý
                    for (const auto& server : g_foundServers) {
                        sf::FloatRect btnRect(475, (float)y, 300, 40);
                        if (btnRect.contains((float)mouseX, (float)mouseY)) {
                            connectToServer(server);
                            break;
                        }
                        y += 50;
                    }
                }
            }
            else if (g_currentState == GameState::LobbyRoom) {
                btnReady.handleEvent(event);
                btnLeave.handleEvent(event);
                if (g_isHost) btnStartGame.handleEvent(event);

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (g_lobbyManager) {
                        int mx = event.mouseButton.x;
                        int my = event.mouseButton.y;

                        int y = 100;
                        for (const auto& p : g_lobbyManager->players()) {
                            sf::FloatRect colorBoxRect(50, (float)y + 5, 30, 30);
                            if (p.id == g_lobbyManager->selfId()) {
                                if (colorBoxRect.contains((float)mx, (float)my)) {
                                    int nextColor = (p.colorIndex + 1) % 4;
                                    g_lobbyManager->changeColor(nextColor);
                                }
                            }
                            y += 40;
                        }
                    }
                }
            }
        }

        // --- DRAW (ÇÝZÝM) ---
        window.clear(sf::Color(40, 40, 50));

        if (g_currentState == GameState::Menu && hasBg) {
            window.draw(bgSprite);
            window.draw(gameNameSprite);
        }
        else if (g_currentState == GameState::LobbySelection && hasLobbySelectionBg) {
            window.draw(lobbySelectionBgSprite);
        }
        else if (g_currentState == GameState::LobbyRoom && hasLobbyBg) {
            window.draw(lobbyBgSprite);
        }

        // UI ÇÝZÝMÝ
        if (g_currentState == GameState::Menu) {
            mainMenu.draw(window);

            // Name Input
            if (g_isEnteringName) {
                sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
                overlay.setFillColor(sf::Color(0, 0, 0, 220));
                window.draw(overlay);

                sf::Text prompt("Kullanici Adinizi Girin:\n(Enter ile onayla)", font, 24);
                sf::FloatRect textRect = prompt.getLocalBounds();
                prompt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                prompt.setPosition(WIN_W / 2.0f, WIN_H / 2.0f - 50);
                window.draw(prompt);

                sf::Text inputDisplay(g_nameInputString + "_", font, 36);
                inputDisplay.setFillColor(sf::Color::Cyan);
                sf::FloatRect inputRect = inputDisplay.getLocalBounds();
                inputDisplay.setOrigin(inputRect.left + inputRect.width / 2.0f, inputRect.top + inputRect.height / 2.0f);
                inputDisplay.setPosition(WIN_W / 2.0f, WIN_H / 2.0f + 20);
                window.draw(inputDisplay);
            }
        }
        else if (g_currentState == GameState::LobbySelection) {
            selectionMenu.draw(window);

            sf::Text header("BULUNAN LOBILER", font, 24);
            header.setPosition(475, 75);
            header.setStyle(sf::Text::Bold | sf::Text::Underlined);
            header.setFillColor(sf::Color(240, 240, 240));
            window.draw(header);

            if (g_foundServers.empty()) {
                sf::Text info("Sunucu araniyor veya yok...\n'Lobi Ara' butonuna basin.", font, 18);
                info.setPosition(480, 130);
                info.setFillColor(sf::Color(200, 200, 200));
                window.draw(info);
            }
            else {
                int y = 150;
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                for (const auto& server : g_foundServers) {
                    sf::RectangleShape row(sf::Vector2f(300, 40));
                    row.setPosition(475, (float)y);

                    if (row.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y))
                        row.setFillColor(sf::Color(80, 80, 120, 200));
                    else
                        row.setFillColor(sf::Color(50, 50, 70, 200));

                    window.draw(row);

                    sf::Text t(server.name, font, 20);
                    t.setPosition(485, (float)y + 8);
                    t.setFillColor(sf::Color::White);
                    window.draw(t);

                    sf::Text ip(server.address.toString(), font, 14);
                    ip.setPosition(675, (float)y + 12);
                    ip.setFillColor(sf::Color(200, 200, 200));
                    window.draw(ip);
                    y += 50;
                }
            }

            if (g_isEnteringIP) {
                sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
                overlay.setFillColor(sf::Color(0, 0, 0, 180));
                window.draw(overlay);

                sf::Text prompt("Baglanilacak IP Girin:\n(Enter ile onayla)", font, 24);
                prompt.setPosition(WIN_W / 2.0f - 150, WIN_H / 2.0f - 50);
                window.draw(prompt);

                sf::Text inputDisplay(g_ipInputString + "_", font, 36);
                inputDisplay.setFillColor(sf::Color::Yellow);
                inputDisplay.setPosition(WIN_W / 2.0f - 150, WIN_H / 2.0f + 20);
                window.draw(inputDisplay);
            }
        }
        else if (g_currentState == GameState::LobbyRoom) {
            sf::Text title(g_isHost ? "Lobi (HOST)" : "Lobi (CLIENT)", font, 30);
            title.setPosition(50, 50);
            title.setFillColor(sf::Color::White);
            window.draw(title);

            if (g_lobbyManager) {
                int y = 100;
                for (const auto& p : g_lobbyManager->players()) {
                    sf::RectangleShape colorBox(sf::Vector2f(30, 30));
                    colorBox.setPosition(50, (float)y + 5);
                    colorBox.setFillColor(getColorFromIndex(p.colorIndex));

                    if (p.id == g_lobbyManager->selfId()) {
                        colorBox.setOutlineThickness(3);
                        colorBox.setOutlineColor(sf::Color::White);
                    }
                    else {
                        colorBox.setOutlineThickness(1);
                        colorBox.setOutlineColor(sf::Color(100, 100, 100));
                    }
                    window.draw(colorBox);

                    std::string line = std::to_string(p.id) + " | " + p.name + (p.ready ? " [HAZIR]" : " [Bekliyor]");
                    sf::Text pt(line, font, 24);
                    pt.setPosition(90, (float)y + 2);
                    pt.setFillColor(p.ready ? sf::Color::Green : sf::Color::White);
                    window.draw(pt);

                    if (p.id == g_lobbyManager->selfId()) {
                        sf::Text hint("(Tikla -> Renk Degis)", font, 14);
                        hint.setPosition(450, (float)y + 10);
                        hint.setFillColor(sf::Color(200, 200, 200));
                        window.draw(hint);
                    }
                    y += 40;
                }
            }
            if (g_isHost) {
                sf::IpAddress localIP = sf::IpAddress::getLocalAddress();
                sf::Text ipText("Sunucu IP: " + localIP.toString(), font, 20);
                sf::FloatRect textRect = ipText.getLocalBounds();
                float xPos = window.getSize().x - textRect.width - 60;
                float yPos = 450;
                ipText.setPosition(xPos, yPos);
                ipText.setFillColor(sf::Color::Yellow);
                window.draw(ipText);
            }
            btnReady.draw(window);
            btnLeave.draw(window);
            if (g_isHost) btnStartGame.draw(window);
        }

        // --- OYUN BAÞLATMA ---
        else if (g_currentState == GameState::Playing) {
            window.close();

            try {
                std::cout << "[MAIN] Oyun baslatiliyor..." << std::endl;

                // 1. Önceki að servislerini durdur
                g_netManager.discovery()->stop();
                g_netManager.stop();
                if (g_lobbyManager) { delete g_lobbyManager; g_lobbyManager = nullptr; }

                // --- ÝÞTE ÇÖZÜM BURASI ---
                std::cout << "[MAIN] Portun bosa cikmasi bekleniyor..." << std::endl;
                // Portun tamamen boþa düþmesi için yarým saniye bekle
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                Game game(g_isHost, g_targetIP);
                game.run();
            }
            catch (const std::exception& e) {
                std::cerr << "KRITIK HATA: " << e.what() << std::endl;
                return -1;
            }
            return 0;
}

        window.display();
    }

    if (g_lobbyManager) delete g_lobbyManager;
    return 0;
}