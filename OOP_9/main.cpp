// main.cpp - RTS Network: Görsel Butonlar, Siyah Yazý, Doðru Hizalama

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
#include "NetworkManager.h"
#include "LobbyManager.h"
#include "NetServer.h"
#include "NetClient.h"
#include "LANDiscovery.h"
#include "UIManager.h" 

// --- Global Sabitler ---
const unsigned short GAME_PORT = 54000;
const unsigned short DISCOVERY_PORT = 50000;
const std::string SERVER_NAME = "RTS Host Lobby";

// --- Oyun Durumlarý ---
enum class GameState {
    MAIN_MENU,
    LOBBY_SELECTION,
    LOBBY_ROOM,
    GAME_PLAYING
};

// --- Global Yöneticiler ---
NetworkManager g_netManager;
LobbyManager* g_lobbyManager = nullptr;
bool g_isHost = false;
GameState g_currentState = GameState::MAIN_MENU;

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
    g_lobbyManager->start(1, "Host Oyuncu");
    g_isHost = true;

    g_netManager.server()->setOnPacket([](uint64_t clientId, sf::Packet& pkt) {
        if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(clientId, pkt);
        });

    g_netManager.server()->setOnClientDisconnected([](uint64_t clientId) {
        if (g_lobbyManager) g_lobbyManager->removePlayer(clientId);
        });

    g_lobbyManager->setOnGameStart([]() { g_currentState = GameState::GAME_PLAYING; });
    g_currentState = GameState::LOBBY_ROOM;
}

// 2. SUNUCUYA BAÐLANMA
void connectToServer(const LANDiscovery::ServerInfo& info) {
    g_netManager.discovery()->stop();

    if (g_netManager.startClient(info.address.toString(), info.port)) {
        g_lobbyManager = new LobbyManager(&g_netManager, false);
        std::string randomName = "Oyuncu_" + std::to_string(rand() % 100);
        g_lobbyManager->start(0, randomName);

        g_netManager.client()->setOnPacket([](sf::Packet& pkt) {
            if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(0, pkt);
            });

        g_netManager.client()->setOnDisconnected([]() {
            if (g_lobbyManager) { delete g_lobbyManager; g_lobbyManager = nullptr; }
            g_currentState = GameState::MAIN_MENU;
            });

        g_lobbyManager->setOnGameStart([]() { g_currentState = GameState::GAME_PLAYING; });
        g_currentState = GameState::LOBBY_ROOM;
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
        sf::Packet pkt; pkt << static_cast<sf::Int32>(4);
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
    g_currentState = GameState::MAIN_MENU;
}

// --- MAIN ---
int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    g_netManager.setLogger([](const std::string& msg) { /* std::cout << msg << std::endl; */ });

    sf::RenderWindow window(sf::VideoMode(900, 600), "RTS Proje - Visual Buttons");
    window.setFramerateLimit(60);

    // --- Font Yükleme ---
    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "HATA: assets/arial.ttf yok." << std::endl;
    }

    // --- GÖRSEL YÜKLEME ---
    sf::Texture btnTexture;
    if (!btnTexture.loadFromFile("assets/button.png")) {
        std::cerr << "HATA: assets/button.png bulunamadi!" << std::endl;
    }

    // --- UI Buttonlar ---
    // PÜF NOKTA: Yazýlarýn ortalanmasý için önce setSize() ile boyut veriyoruz.
    // Görseli atamak için setTexture() kullanýyoruz.

    // 1. Ana Menü
    UIPanel mainMenu({ 400, 300 }, { 250, 150 });

    UIButton btnStart;
    btnStart.setPosition(325, 200);
    btnStart.setSize(250, 50);             // 1. Önce Boyut (Yazý hizalamasý için)
    btnStart.setTexture(btnTexture, 250, 50); // 2. Sonra Resim
    btnStart.setText("Oyna", font);
    btnStart.setCallback([&]() { g_currentState = GameState::LOBBY_SELECTION; });

    UIButton btnExit;
    btnExit.setPosition(325, 270);
    btnExit.setSize(250, 50);
    btnExit.setTexture(btnTexture, 250, 50);
    btnExit.setText("Cikis", font);
    btnExit.setCallback([&]() { window.close(); });

    mainMenu.addButton(btnStart);
    mainMenu.addButton(btnExit);

    // 2. Seçim Ekraný
    UIPanel selectionMenu({ 400, 450 }, { 50, 50 });

    UIButton btnCreate;
    btnCreate.setPosition(125, 100);
    btnCreate.setSize(250, 50);
    btnCreate.setTexture(btnTexture, 250, 50);
    btnCreate.setText("Lobi Kur (Host)", font);
    btnCreate.setCallback(startHost);

    UIButton btnSearch;
    btnSearch.setPosition(125, 170);
    btnSearch.setSize(250, 50);
    btnSearch.setTexture(btnTexture, 250, 50);
    btnSearch.setText("Lobi Ara (Yenile)", font);
    btnSearch.setCallback(startDiscoveryMode);

    UIButton btnBack;
    btnBack.setPosition(125, 400);
    btnBack.setSize(250, 50);
    btnBack.setTexture(btnTexture, 250, 50);
    btnBack.setText("Geri", font);
    btnBack.setCallback([&]() {
        g_netManager.discovery()->stop();
        g_currentState = GameState::MAIN_MENU;
        });

    selectionMenu.addButton(btnCreate);
    selectionMenu.addButton(btnSearch);
    selectionMenu.addButton(btnBack);

    // 3. Lobi Odasý Butonlarý
    UIButton btnReady;
    btnReady.setPosition(350, 500);
    btnReady.setSize(200, 50);
    btnReady.setTexture(btnTexture, 200, 50);
    btnReady.setText("HAZIR OL", font);
    btnReady.setCallback([]() { if (g_lobbyManager) g_lobbyManager->toggleReady(!isSelfReady()); });

    UIButton btnStartGame;
    btnStartGame.setPosition(600, 500);
    btnStartGame.setSize(200, 50);
    btnStartGame.setTexture(btnTexture, 200, 50);
    btnStartGame.setText("BASLAT", font);
    btnStartGame.setCallback([]() { if (g_lobbyManager && g_isHost) g_lobbyManager->startGame(); });

    UIButton btnLeave;
    btnLeave.setPosition(50, 500);
    btnLeave.setSize(100, 40);
    btnLeave.setTexture(btnTexture, 100, 40);
    btnLeave.setText("Ayril", font);
    btnLeave.setCallback(leaveLobby);


    // --- OYUN DÖNGÜSÜ ---
    sf::Clock dtClock;
    sf::Clock discoveryTimer;

    while (window.isOpen()) {
        float dt = dtClock.restart().asSeconds();
        g_netManager.update(dt);

        // Ping Atma
        if (!g_isHost && g_lobbyManager == nullptr && g_currentState == GameState::LOBBY_SELECTION) {
            if (discoveryTimer.getElapsedTime().asSeconds() > 1.0f) {
                g_netManager.discovery()->sendDiscoveryRequest();
                discoveryTimer.restart();
            }
        }

        // Görsel Update (Hazýr Butonu Metni)
        if (g_currentState == GameState::LOBBY_ROOM) {
            btnReady.setText(isSelfReady() ? "VAZGEC" : "HAZIR OL", font);
        }

        // Event
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                leaveLobby();
                window.close();
            }

            if (g_currentState == GameState::MAIN_MENU) mainMenu.handleEvent(event);
            else if (g_currentState == GameState::LOBBY_SELECTION) {
                selectionMenu.handleEvent(event);

                // SUNUCU LÝSTESÝNE TIKLAMA
                if (!g_isHost && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;

                    int y = 100;
                    for (const auto& server : g_foundServers) {
                        sf::FloatRect btnRect(500, y, 300, 40);
                        if (btnRect.contains((float)mouseX, (float)mouseY)) {
                            connectToServer(server);
                            break;
                        }
                        y += 50;
                    }
                }
            }
            else if (g_currentState == GameState::LOBBY_ROOM) {
                btnReady.handleEvent(event);
                btnLeave.handleEvent(event);
                if (g_isHost) btnStartGame.handleEvent(event);

                // --- RENK KUTUCUÐUNA TIKLAMA ---
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (g_lobbyManager) {
                        int mx = event.mouseButton.x;
                        int my = event.mouseButton.y;

                        int y = 100;
                        for (const auto& p : g_lobbyManager->players()) {
                            sf::FloatRect colorBoxRect(50, y + 5, 30, 30);
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

        // Draw
        window.clear(sf::Color(40, 40, 50));

        if (g_currentState == GameState::MAIN_MENU) mainMenu.draw(window);
        else if (g_currentState == GameState::LOBBY_SELECTION) {
            selectionMenu.draw(window);

            sf::Text header("BULUNAN LOBILER", font, 24);
            header.setPosition(500, 50);
            header.setStyle(sf::Text::Bold | sf::Text::Underlined);
            window.draw(header);

            if (g_foundServers.empty()) {
                sf::Text info("Sunucu araniyor veya yok...\n'Lobi Ara' butonuna basin.", font, 18);
                info.setPosition(500, 100);
                info.setFillColor(sf::Color(150, 150, 150));
                window.draw(info);
            }
            else {
                int y = 100;
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                for (const auto& server : g_foundServers) {
                    sf::RectangleShape row(sf::Vector2f(300, 40));
                    row.setPosition(500, y);
                    if (row.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y))
                        row.setFillColor(sf::Color(70, 70, 100));
                    else
                        row.setFillColor(sf::Color(50, 50, 60));
                    window.draw(row);

                    sf::Text t(server.name, font, 20);
                    t.setPosition(510, y + 8);
                    window.draw(t);

                    sf::Text ip(server.address.toString(), font, 14);
                    ip.setPosition(700, y + 12);
                    ip.setFillColor(sf::Color(180, 180, 180));
                    window.draw(ip);
                    y += 50;
                }
            }
        }
        else if (g_currentState == GameState::LOBBY_ROOM) {
            sf::Text title(g_isHost ? "Lobi (HOST)" : "Lobi (CLIENT)", font, 30);
            title.setPosition(50, 50); window.draw(title);

            if (g_lobbyManager) {
                int y = 100;
                for (const auto& p : g_lobbyManager->players()) {
                    sf::RectangleShape colorBox(sf::Vector2f(30, 30));
                    colorBox.setPosition(50, y + 5);
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
                    pt.setPosition(90, y + 2);
                    pt.setFillColor(p.ready ? sf::Color::Green : sf::Color::White);
                    window.draw(pt);

                    if (p.id == g_lobbyManager->selfId()) {
                        sf::Text hint("(Tikla -> Renk Degis)", font, 14);
                        hint.setPosition(450, y + 10);
                        hint.setFillColor(sf::Color(200, 200, 200));
                        window.draw(hint);
                    }
                    y += 40;
                }
            }

            // Butonlarý Çiz
            btnReady.draw(window);
            btnLeave.draw(window);
            if (g_isHost) btnStartGame.draw(window);
        }
        else if (g_currentState == GameState::GAME_PLAYING) {
            sf::Text t("OYUN BASLADI!", font, 50); t.setPosition(250, 250); window.draw(t);
        }

        window.display();
    }

    if (g_lobbyManager) delete g_lobbyManager;
    return 0;
}