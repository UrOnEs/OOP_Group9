// main.cpp - RTS Network: Server Browser + Liste Üzerinden Renk Seçimi
// Özellikler:
// 1. Sunucularý listeleme ve seçerek katýlma.
// 2. LÝSTE ÜZERÝNDEKÝ KUTUCUÐA týklayarak renk deðiþtirme.
// 3. Kendi kutucuðumuzu vurgulama.

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
    LOBBY_SELECTION, // Host Kur veya Lobi Ara ekraný
    LOBBY_ROOM,      // Bekleme odasý
    GAME_PLAYING     // Oyun içi
};

// --- Global Yöneticiler ---
NetworkManager g_netManager;
LobbyManager* g_lobbyManager = nullptr;
bool g_isHost = false;
GameState g_currentState = GameState::MAIN_MENU;

// Bulunan Sunucularýn Listesi
std::vector<LANDiscovery::ServerInfo> g_foundServers;

// --- Yardýmcý Fonksiyonlar ---

// Renk Ýndeksine Göre SFML Rengi Döndürür
sf::Color getColorFromIndex(int index) {
    switch (index) {
    case 0: return sf::Color::Red;              // Kýrmýzý
    case 1: return sf::Color::Blue;             // Mavi
    case 2: return sf::Color::Green;            // Yeþil
    case 3: return sf::Color(160, 32, 240);     // Mor
    default: return sf::Color::White;
    }
}

// Kendimizin hazýr olup olmadýðýný kontrol eder
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

    if (!g_netManager.startServer(GAME_PORT)) {
        std::cerr << "[ERROR] Sunucu baslatilamadi!" << std::endl;
        return;
    }

    g_netManager.discovery()->startServer(GAME_PORT, DISCOVERY_PORT, SERVER_NAME);

    g_lobbyManager = new LobbyManager(&g_netManager, true);
    g_lobbyManager->start(1, "Host Oyuncu");
    g_isHost = true;

    // A. Paket Geldiðinde
    g_netManager.server()->setOnPacket([](uint64_t clientId, sf::Packet& pkt) {
        if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(clientId, pkt);
        });

    // B. Ýstemci Koptuðunda
    g_netManager.server()->setOnClientDisconnected([](uint64_t clientId) {
        std::cout << "[SERVER] Istemci ID: " << clientId << " koptu." << std::endl;
        if (g_lobbyManager) {
            g_lobbyManager->removePlayer(clientId);
        }
        });

    // C. Oyun Baþlama
    g_lobbyManager->setOnGameStart([]() {
        std::cout << "Oyun Basliyor!" << std::endl;
        g_currentState = GameState::GAME_PLAYING;
        });

    g_currentState = GameState::LOBBY_ROOM;
}

// 2. SUNUCUYA BAÐLANMA
void connectToServer(const LANDiscovery::ServerInfo& info) {
    std::cout << "[UI] Secilen sunucuya baglaniliyor: " << info.name << std::endl;

    g_netManager.discovery()->stop();

    if (g_netManager.startClient(info.address.toString(), info.port)) {
        g_lobbyManager = new LobbyManager(&g_netManager, false);

        std::string randomName = "Oyuncu_" + std::to_string(rand() % 100);
        g_lobbyManager->start(0, randomName);

        // Paket Handler
        g_netManager.client()->setOnPacket([](sf::Packet& pkt) {
            if (g_lobbyManager) g_lobbyManager->handleIncomingPacket(0, pkt);
            });

        // Host Kapanýrsa Menüye Dön
        g_netManager.client()->setOnDisconnected([]() {
            std::cout << "[CLIENT] Baglanti koptu! Menuye donuluyor..." << std::endl;
            if (g_lobbyManager) {
                delete g_lobbyManager;
                g_lobbyManager = nullptr;
            }
            g_currentState = GameState::MAIN_MENU;
            });

        g_lobbyManager->setOnGameStart([]() {
            std::cout << "Host oyunu baslatti!" << std::endl;
            g_currentState = GameState::GAME_PLAYING;
            });

        g_currentState = GameState::LOBBY_ROOM;
    }
    else {
        std::cerr << "[ERROR] Baglanti basarisiz!" << std::endl;
    }
}

// 3. ARAMA MODUNU BAÞLAT
void startDiscoveryMode() {
    std::cout << "[UI] Sunucu arama modu baslatildi..." << std::endl;
    g_foundServers.clear();
    g_netManager.discovery()->stop();
    g_netManager.discovery()->startClient(DISCOVERY_PORT);
    g_isHost = false;

    g_netManager.discovery()->setOnServerFound([](const LANDiscovery::ServerInfo& info) {
        bool exists = false;
        for (const auto& s : g_foundServers) {
            if (s.address == info.address && s.port == info.port) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            std::cout << "[DISCOVERY] Listeye eklendi: " << info.name << std::endl;
            g_foundServers.push_back(info);
        }
        });
}

// 4. LOBÝDEN AYRILMA
void leaveLobby() {
    std::cout << "[UI] Lobiden ayrilma..." << std::endl;

    g_netManager.discovery()->stop();

    if (!g_isHost && g_netManager.client() && g_netManager.client()->isConnected()) {
        sf::Packet pkt;
        pkt << static_cast<sf::Int32>(4); // LeaveLobby
        g_netManager.client()->send(pkt);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        g_netManager.client()->disconnect();
    }

    if (g_isHost) {
        if (g_lobbyManager) g_lobbyManager->closeLobby();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (g_netManager.server()) g_netManager.server()->stop();
    }

    if (g_lobbyManager) {
        delete g_lobbyManager;
        g_lobbyManager = nullptr;
    }

    g_isHost = false;
    g_foundServers.clear();
    g_currentState = GameState::MAIN_MENU;
}

// --- MAIN ---
int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    g_netManager.setLogger([](const std::string& msg) { /* std::cout << msg << std::endl; */ });

    sf::RenderWindow window(sf::VideoMode(900, 600), "RTS Proje - Smart UI");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "HATA: assets/arial.ttf yok." << std::endl;
    }

    // --- UI Buttonlar ---

    // 1. Ana Menü
    UIPanel mainMenu({ 400, 300 }, { 250, 150 });
    UIButton btnStart({ 250, 50 }, { 325, 200 }); btnStart.setText("Oyna", font);
    UIButton btnExit({ 250, 50 }, { 325, 270 }); btnExit.setText("Cikis", font);

    btnStart.setCallback([&]() {
        g_currentState = GameState::LOBBY_SELECTION;
        });
    btnExit.setCallback([&]() { window.close(); });
    mainMenu.addButton(btnStart); mainMenu.addButton(btnExit);

    // 2. Seçim Ekraný
    UIPanel selectionMenu({ 400, 450 }, { 50, 50 }); // Sol tarafa
    UIButton btnCreate({ 250, 50 }, { 125, 100 }); btnCreate.setText("Lobi Kur (Host)", font);
    UIButton btnSearch({ 250, 50 }, { 125, 170 }); btnSearch.setText("Lobi Ara (Yenile)", font);
    UIButton btnBack({ 250, 50 }, { 125, 400 }); btnBack.setText("Geri", font);

    btnCreate.setCallback(startHost);
    btnSearch.setCallback(startDiscoveryMode);
    btnBack.setCallback([&]() {
        g_netManager.discovery()->stop();
        g_currentState = GameState::MAIN_MENU;
        });
    selectionMenu.addButton(btnCreate); selectionMenu.addButton(btnSearch); selectionMenu.addButton(btnBack);

    // 3. Lobi Odasý Butonlarý
    UIButton btnReady({ 200, 50 }, { 350, 500 }); btnReady.setText("HAZIR OL", font);
    btnReady.setCallback([]() { if (g_lobbyManager) g_lobbyManager->toggleReady(!isSelfReady()); });

    UIButton btnStartGame({ 200, 50 }, { 600, 500 }); btnStartGame.setText("BASLAT", font);
    btnStartGame.setCallback([]() { if (g_lobbyManager && g_isHost) g_lobbyManager->startGame(); });

    UIButton btnLeave({ 100, 40 }, { 50, 500 }); btnLeave.setText("Ayril", font);
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

                // --- RENK KUTUCUÐUNA TIKLAMA KONTROLÜ ---
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (g_lobbyManager) {
                        int mx = event.mouseButton.x;
                        int my = event.mouseButton.y;

                        // Oyuncu listesi pozisyonlarýný tekrar hesaplayýp týklama kontrolü yapýyoruz
                        int y = 100;
                        for (const auto& p : g_lobbyManager->players()) {
                            // Kutucuk konumu (Çizimdeki ile ayný olmalý)
                            // Çizimde: setPosition(50, y + 5), Boyut 30x30
                            sf::FloatRect colorBoxRect(50, y + 5, 30, 30);

                            // Eðer bu BÝZÝM oyuncumuzsa ve kutucuða týklandýysa
                            if (p.id == g_lobbyManager->selfId()) {
                                if (colorBoxRect.contains((float)mx, (float)my)) {
                                    int nextColor = (p.colorIndex + 1) % 4; // Döngüsel renk deðiþimi
                                    g_lobbyManager->changeColor(nextColor);
                                    std::cout << "[UI] Renk degisti: " << nextColor << std::endl;
                                }
                            }
                            y += 40; // Sonraki satýr
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

                    // 1. Renk Kutucuðu (Týklanabilir)
                    // Biraz daha büyük yapalým: 30x30
                    sf::RectangleShape colorBox(sf::Vector2f(30, 30));
                    colorBox.setPosition(50, y + 5);
                    colorBox.setFillColor(getColorFromIndex(p.colorIndex));

                    // Eðer bizsek vurgula (Outline)
                    if (p.id == g_lobbyManager->selfId()) {
                        colorBox.setOutlineThickness(3);
                        colorBox.setOutlineColor(sf::Color::White); // Biz olduðumuzu belli et
                    }
                    else {
                        colorBox.setOutlineThickness(1);
                        colorBox.setOutlineColor(sf::Color(100, 100, 100));
                    }

                    window.draw(colorBox);

                    // 2. Ýsim ve Durum
                    std::string line = std::to_string(p.id) + " | " + p.name + (p.ready ? " [HAZIR]" : " [Bekliyor]");
                    sf::Text pt(line, font, 24);
                    pt.setPosition(90, y + 2); // Kutu büyüdüðü için biraz daha saða
                    pt.setFillColor(p.ready ? sf::Color::Green : sf::Color::White);
                    window.draw(pt);

                    // Bizim ismimizin yanýna küçük bir ipucu (Opsiyonel)
                    if (p.id == g_lobbyManager->selfId()) {
                        sf::Text hint("(Rengi degistirmek icin kutuya tikla)", font, 14);
                        hint.setPosition(450, y + 10);
                        hint.setFillColor(sf::Color(200, 200, 200));
                        window.draw(hint);
                    }

                    y += 40;
                }
            }
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