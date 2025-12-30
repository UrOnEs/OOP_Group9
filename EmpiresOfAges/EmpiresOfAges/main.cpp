

// main.cpp
#include "Game/Game.h"
#include <iostream>

// main.cpp - RTS Network: Hizalanm?? Scroll Butonlar

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <functional>
#include <chrono> 
#include <thread>

// --- Kendi Header Dosyalar?n?z ---
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
bool g_isEnteringIP = false;
std::string g_ipInputString = "";
bool g_isEnteringName = false;
std::string g_playerCurrentName = "Player";

// --- Oyun Durumlar? (GameState'e ta??nd?)---


// --- Global Yöneticiler ---
NetworkManager g_netManager;
LobbyManager* g_lobbyManager = nullptr;
bool g_isHost = false;
GameState g_currentState = GameState::Menu;

std::vector<LANDiscovery::ServerInfo> g_foundServers;

// --- Yard?mc? Fonksiyonlar ---

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

// 1. HOST BA?LATMA
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

// 2. SUNUCUYA BA?LANMA
void connectToServer(const LANDiscovery::ServerInfo& info) {
    g_netManager.discovery()->stop();

    if (g_netManager.startClient(info.address.toString(), info.port)) {
        g_lobbyManager = new LobbyManager(&g_netManager, false);
        std::string randomName = "Oyuncu_" + std::to_string(rand() % 100);
        if (g_playerCurrentName != "Player") {
            g_lobbyManager->start(0, g_playerCurrentName);
        }
        else
            g_lobbyManager->start(0, randomName);

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
// main.cpp içine ekleyin
void connectToDirectIP(const std::string& ipStr) {
    g_netManager.discovery()->stop();

    // Port'u GAME_PORT (54000) olarak sabit al?yoruz
    if (g_netManager.startClient(ipStr, GAME_PORT)) {
        g_lobbyManager = new LobbyManager(&g_netManager, false);
        std::string randomName = "Oyuncu_" + std::to_string(rand() % 100);
        g_lobbyManager->start(0, randomName);

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
    g_currentState = GameState::Menu;
}

// --- MAIN ---
int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    g_netManager.setLogger([](const std::string& msg) { /* std::cout << msg << std::endl; */ });

    // Pencere Boyutu
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
        std::cerr << "HATA: asset bulunamad?" << std::endl;
    }

    // --- 2. Button Texture Yükleme ---
    sf::Texture btnTexture;
    if (!btnTexture.loadFromFile("assets/MenuUI/button2.png")) {
        std::cerr << "HATA: assets/button2.png bulunamadi!" << std::endl;
    }
    // Pürüzsüz görünüm için (Gerekirse açabilirsin)
    // btnTexture.setSmooth(true); 

    // --- 3. BACKGROUND (ARKA PLAN) YÜKLEME ---
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

    // E?er background.png yoksa .jpg dene
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

    //Oyunun ismini koymaya çal???yom
    sf::Texture gameNameTexture;
    sf::Sprite gameNameSprite;
    if (gameNameTexture.loadFromFile("assets/MenuUI/Empire of Ages1.png")) {
        gameNameSprite.setTexture(gameNameTexture);
        gameNameSprite.setOrigin(gameNameSprite.getLocalBounds().width / 2.0f, gameNameSprite.getLocalBounds().height / 2.0f);
        gameNameSprite.setPosition(450.0f, 120.0f);
        gameNameSprite.setScale({ 0.25f,0.25f });
    }

    // --- UI Buttonlar ---
    // BUTON AYARLARI: 
    // Scroll resmi oldu?u için biraz daha geni? ve yüksek yap?yoruz (280x65).
    // Aralar?ndaki bo?lu?u (Y ekseni) e?itliyoruz.

    // 1. Ana Menü


    bool g_isEnteringName = true;
    std::string g_nameInputString = "";

    UIPanel mainMenu({ 400, 300 }, { 250, 250 });


    UIButton btnStart;
    btnStart.setPosition(310, 300);       // Ortalamak için ayarland?
    btnStart.setSize(280, 65);            // Par?ömen boyutu
    btnStart.setTexture(btnTexture, 280, 65);
    btnStart.setText("Play", menuFont);
    btnStart.setCallback([&]() { g_currentState = GameState::LobbySelection; });

    UIButton btnExit;
    btnExit.setPosition(310, 380);        // 80 birim a?a??ya
    btnExit.setSize(280, 65);
    btnExit.setTexture(btnTexture, 280, 65);
    btnExit.setText("Exit", menuFont);
    btnExit.setCallback([&]() { window.close(); });

    mainMenu.addButton(btnStart);
    mainMenu.addButton(btnExit);


    if (g_isEnteringName) {
        sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text prompt("Name:\n(Enter ile onayla)", font, 24);
        prompt.setPosition(WIN_W / 2 - 150, WIN_H / 2 - 50);
        window.draw(prompt);

        sf::Text inputDisplay(g_nameInputString + "_", font, 36);
        inputDisplay.setFillColor(sf::Color::Yellow);
        inputDisplay.setPosition(WIN_W / 2 - 150, WIN_H / 2 + 20);
        window.draw(inputDisplay);
    }

    // 2. Seçim Ekran? (Lobby Selection)
    // SOL MENÜ H?ZALAMASI BURADA YAPILDI

    UIPanel selectionMenu({ 0, 0 }, { 50, 50 });

    UIButton btnCreate;
    btnCreate.setPosition(25, 120);          // ?lk Buton
    btnCreate.setSize(280, 65);
    btnCreate.setTexture(btnTexture, 280, 65);
    btnCreate.setText("Lobi Kur (Host)", menuFont);
    btnCreate.setCallback(startHost);

    UIButton btnSearch;
    btnSearch.setPosition(25, 200);          // 80 birim alt?
    btnSearch.setSize(280, 65);
    btnSearch.setTexture(btnTexture, 280, 65);
    btnSearch.setText("Lobi Ara (Yenile)", menuFont);
    btnSearch.setCallback(startDiscoveryMode);

    UIButton btnBack;
    btnBack.setPosition(25, 280);            // 80 birim alt? (Art?k en altta de?il, liste ?eklinde)
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
        g_ipInputString = ""; // Kutuyu temizle
        });

    selectionMenu.addButton(btnCreate);
    selectionMenu.addButton(btnSearch);
    selectionMenu.addButton(btnBack);
    selectionMenu.addButton(btnDirectConnect);

    // 3. Lobi Odas? Butonlar?
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
    btnLeave.setSize(120, 50); // Ç?k?? butonu biraz daha küçük olabilir
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
            //NAME Input
            if (g_isEnteringName) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) { // Backspace (Silme)
                        if (!g_nameInputString.empty()) g_nameInputString.pop_back();
                    }
                    else if (event.text.unicode == 13) { // Enter (Onaylama)
                        if (!g_nameInputString.empty()) {
                            g_playerCurrentName = g_nameInputString; // ?smi kaydet
                            g_isEnteringName = false; // Paneli kapat
                            std::cout << "[UI] Isim ayarlandi: " << g_playerCurrentName << std::endl;
                        }
                    }
                    else if (event.text.unicode < 128) { // ASCII Karakterler
                        // ?sim çok uzun olmas?n diye s?n?r koyabilirsin (örn: 12 karakter)
                        if (g_nameInputString.length() < 12) {
                            g_nameInputString += static_cast<char>(event.text.unicode);
                        }
                    }
                }
            }
            //IP Input
            if (g_isEnteringIP) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) { // Backspace
                        if (!g_ipInputString.empty()) g_ipInputString.pop_back();
                    }
                    else if (event.text.unicode == 13) { // Enter
                        if (!g_ipInputString.empty()) {
                            connectToDirectIP(g_ipInputString);
                            g_isEnteringIP = false;
                        }
                    }
                    else if (event.text.unicode < 128) { // ASCII karakterler (Say? ve nokta)
                        g_ipInputString += static_cast<char>(event.text.unicode);
                    }
                }
                // IP girerken di?er butonlara bas?lmas?n? engellemek için continue diyebiliriz
                if (event.type == sf::Event::MouseButtonPressed) g_isEnteringIP = false; // Bo?a t?klarsa kapat
            }
            if (event.type == sf::Event::Closed) {
                leaveLobby();
                window.close();
            }

            if (g_currentState == GameState::Menu) mainMenu.handleEvent(event);
            else if (g_currentState == GameState::LobbySelection) {
                selectionMenu.handleEvent(event);

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

        // --- DRAW (Ç?Z?M) ---
        window.clear(sf::Color(40, 40, 50));

        // 1. ARKA PLAN
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

        // 2. UI Ç?Z?M?
        if (g_currentState == GameState::Menu) {
            mainMenu.draw(window);
        }
        else if (g_currentState == GameState::LobbySelection) {
            selectionMenu.draw(window);

            sf::Text header("BULUNAN LOBILER", font, 24);
            header.setPosition(475, 75);
            header.setStyle(sf::Text::Bold | sf::Text::Underlined);
            // Yaz? rengini par?ömen/tahta uyumlu yapal?m (Aç?k renk)
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
                    row.setPosition(475, y);

                    // Liste elemanlar? üzerine gelince hafif ayd?nlans?n
                    if (row.getGlobalBounds().contains((float)mousePos.x, (float)mousePos.y))
                        row.setFillColor(sf::Color(80, 80, 120, 200));
                    else
                        row.setFillColor(sf::Color(50, 50, 70, 200)); // Hafif ?effaf

                    window.draw(row);

                    sf::Text t(server.name, font, 20);
                    t.setPosition(485, y + 8);
                    t.setFillColor(sf::Color::White);
                    window.draw(t);

                    sf::Text ip(server.address.toString(), font, 14);
                    ip.setPosition(675, y + 12);
                    ip.setFillColor(sf::Color(200, 200, 200));
                    window.draw(ip);
                    y += 50;
                }

            }
            // main.cpp - Render k?sm?nda LobbySelection blo?unun içine
            if (g_isEnteringIP) {
                sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
                overlay.setFillColor(sf::Color(0, 0, 0, 180));
                window.draw(overlay);

                sf::Text prompt("Baglanilacak IP Girin:\n(Enter ile onayla)", font, 24);
                prompt.setPosition(WIN_W / 2 - 150, WIN_H / 2 - 50);
                window.draw(prompt);

                sf::Text inputDisplay(g_ipInputString + "_", font, 36);
                inputDisplay.setFillColor(sf::Color::Yellow);
                inputDisplay.setPosition(WIN_W / 2 - 150, WIN_H / 2 + 20);
                window.draw(inputDisplay);
            }
            //NameInput
            if (g_isEnteringName) {
                // Arka plan? karart
                sf::RectangleShape overlay(sf::Vector2f(WIN_W, WIN_H));
                overlay.setFillColor(sf::Color(0, 0, 0, 220)); // Arka plan? neredeyse tamamen kapat
                window.draw(overlay);

                // Ba?l?k
                sf::Text prompt("Kullanici Adinizi Girin:\n(Enter ile onayla)", font, 24);
                sf::FloatRect textRect = prompt.getLocalBounds();
                prompt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                prompt.setPosition(WIN_W / 2.0f, WIN_H / 2.0f - 50);
                window.draw(prompt);

                // Girilen Yaz?
                sf::Text inputDisplay(g_nameInputString + "_", font, 36);
                inputDisplay.setFillColor(sf::Color::Cyan); // Farkl? renk olsun
                sf::FloatRect inputRect = inputDisplay.getLocalBounds();
                inputDisplay.setOrigin(inputRect.left + inputRect.width / 2.0f, inputRect.top + inputRect.height / 2.0f);
                inputDisplay.setPosition(WIN_W / 2.0f, WIN_H / 2.0f + 20);
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
            if (g_isHost) {
                sf::IpAddress localIP = sf::IpAddress::getLocalAddress();
                sf::Text ipText("Sunucu IP: " + localIP.toString(), font, 20);

                //NEW ADDED IP PLACEMENT
                sf::FloatRect textRect = ipText.getLocalBounds();

                // Matematik: (Pencere Geni?li?i) - (Yaz? Geni?li?i) - (Kenar Bo?lu?u)
                // window.getSize().x = 900
                float xPos = window.getSize().x - textRect.width - 60;
                float yPos = 450; // Sol taraftaki "Lobi (HOST)" yaz?s?yla ayn? hizada olsun

                ipText.setPosition(xPos, yPos);
                ipText.setFillColor(sf::Color::Yellow); // Dikkat çeksin diye sar?
                window.draw(ipText);
            }
            btnReady.draw(window);
            btnLeave.draw(window);
            if (g_isHost) btnStartGame.draw(window);
        }
        else if (g_currentState == GameState::Playing) {
            sf::Text t("OYUN BASLADI!", font, 50);
            try {
                std::cout << "Empires of Ages baslatiliyor..." << std::endl;

                Game game;
                game.run();

            }
            catch (const std::exception& e) {
                std::cerr << "KRITIK HATA: " << e.what() << std::endl;
                return -1;
            }

            return 0;
            t.setPosition(250, 250);
            t.setFillColor(sf::Color::White);
            window.draw(t);
        }


        window.display();
    }

    if (g_lobbyManager) delete g_lobbyManager;
    return 0;
}
