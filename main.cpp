#include "NetworkManager.h"
#include "LobbyManager.h"
#include "NetServer.h"  
#include "NetClient.h"  
#include "LANDiscovery.h" 

#include <iostream>
#include <SFML/Network.hpp>
#include <chrono>
#include <thread>
#include <atomic>
#include <stdexcept>
// #include <Windows.h> // Windows konsolunda T�rk�e karakterler i�in gerekebilir

// --- Global ve Port Tan�mlar� ---
unsigned short GAME_PORT = 54000;
unsigned short DISCOVERY_PORT = 50000;
std::atomic<bool> g_isConnected(false);
std::atomic<bool> g_isRunning(true);

// --- Sunucu Test Fonksiyonu ---
void runServer(NetworkManager& netManager, unsigned short gamePort, unsigned short discoveryPort) {
    const std::string SERVER_NAME = "Host'un LAN Lobisi";
    const std::string HOST_PLAYER_NAME = "Sunucu Host";
    const uint64_t HOST_ID = 1;

    if (!netManager.startServer(gamePort)) {
        std::cerr << "Sunucu ba�lat�lamad�!" << std::endl;
        return;
    }
    netManager.discovery()->startServer(gamePort, discoveryPort, SERVER_NAME);

    LobbyManager lobbyManager(&netManager, true);
    lobbyManager.start(HOST_ID, HOST_PLAYER_NAME);

    std::cout << "\n>>> SUNUCU CALISIYOR (Host ID: " << HOST_ID << ") <<<" << std::endl;

    // --- A�/Lobi Entegrasyonu ---
    netManager.server()->setOnPacket([&lobbyManager](uint64_t clientId, sf::Packet& pkt) {
        lobbyManager.handleIncomingPacket(clientId, pkt);
        });
    netManager.server()->setOnClientConnected([](uint64_t newClientId) {
        std::cout << "[S] Istemci ID " << newClientId << " baglandi. Join Request bekleniyor..." << std::endl;
        });

    // --- Lobi Callback'leri ---
    lobbyManager.setOnPlayerChange([&lobbyManager, HOST_ID]() {
        std::cout << "\n--- S_LOBI DURUMU GUNCEL ---" << std::endl;

        bool hostIsReady = false;

        // T�m oyuncu durumlar�n� listele ve Host'un durumunu kontrol et
        for (const auto& p : lobbyManager.players()) {
            std::cout << "-> ID: " << p.id << ", Ad: " << p.name << ", Hazir: " << (p.ready ? "EVET" : "HAYIR") << std::endl;
            if (p.id == HOST_ID) {
                hostIsReady = p.ready;
            }
        }

        // Host Haz�r De�ilse, Host'u manuel olarak haz�r yap
        if (!hostIsReady) {
            std::cout << "[S] Host Hazir Durumu EVET yapiliyor..." << std::endl;
            lobbyManager.toggleReady(true);
        }

        // Oyun ba�latma kontrol�
        if (lobbyManager.canStartGame()) {
            std::cout << "\n[HOST] TUM OYUNCULAR HAZIR. OYUN BASLATILIYOR!" << std::endl;
            lobbyManager.startGame();
        }
        });

    lobbyManager.setOnGameStart([]() {
        std::cout << "\n!!! S_OYUN BASLAMA SINYALI GONDERILDI !!!" << std::endl;
        g_isRunning = false;
        });

    // Ana D�ng�
    while (g_isRunning) {
        netManager.update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    std::cout << "\nSunucu testi bitti." << std::endl;
}

// --- �stemci Test Fonksiyonu ---
void runClient(NetworkManager& netManager, unsigned short discoveryPort, const std::string& clientName) {
    const uint64_t CLIENT_ID = 0; // ID Sunucu taraf�ndan atanacak

    netManager.discovery()->startClient(discoveryPort);

    std::cout << "\n>>> ISTEMCI CALISIYOR (" << clientName << ") <<<" << std::endl;
    std::cout << ">>> Agda sunucu araniyor... <<<" << std::endl;

    LobbyManager* lobbyManager = nullptr;
    std::atomic<bool> readyCommandSent(false); // Haz�r komutunun bir kez g�nderilmesi i�in

    // LAN Discovery Callback'i Ayarla
    netManager.discovery()->setOnServerFound([&netManager, &lobbyManager, clientName](const LANDiscovery::ServerInfo& info) {
        if (g_isConnected) return;

        if (netManager.startClient(info.address.toString(), info.port)) {
            g_isConnected = true;
            std::cout << "[C] Sunucuya baglanildi: " << info.address.toString() << ":" << info.port << std::endl;

            // 3. LobbyManager'� Client olarak ba�lat
            try {
                lobbyManager = new LobbyManager(&netManager, false);
                lobbyManager->start(CLIENT_ID, clientName);
            }
            catch (const std::exception& e) {
                std::cerr << "Lobby Manager hatasi: " << e.what() << std::endl;
                g_isRunning = false;
                return;
            }

            // 4. A�/Lobi Entegrasyonu
            netManager.client()->setOnPacket([&lobbyManager](sf::Packet& pkt) {
                if (lobbyManager) lobbyManager->handleIncomingPacket(0, pkt);
                });

            // 5. Lobi Callback'leri
            lobbyManager->setOnPlayerChange([lobbyManager]() {
                std::cout << "\n--- C_LOBI DURUMU GUNCEL ---" << std::endl;

                // Lobi g�ncellemelerini ekrana bas
                for (const auto& p : lobbyManager->players()) {
                    std::cout << "-> ID: " << p.id << ", Ad: " << p.name << ", Hazir: " << (p.ready ? "EVET" : "HAYIR") << std::endl;
                }

                });

            lobbyManager->setOnGameStart([]() {
                std::cout << "\n!!! C_OYUN BASLAMA SINYALI ALINDI !!!" << std::endl;
                g_isRunning = false;
                });
        }
        });

    // --- Manuel Haz�r Olma ��in Giri� ��leme Thread'i ---
    std::thread inputThread([&lobbyManager, &readyCommandSent]() {
        std::string input;

        // Thread'in ana d�ng�n�n ba�lamas�n� beklemesi i�in k���k bir gecikme
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::cout << "\n-----------------------------------" << std::endl;
        std::cout << "Lobiye katildiktan sonra hazir olmak icin 'h' yazip ENTER'a basin." << std::endl;
        std::cout << "-----------------------------------" << std::endl;

        while (g_isRunning && std::cin >> input) {
            if (lobbyManager && !readyCommandSent && (input == "h" || input == "H")) {
                std::cout << "[C] Hazir komutu gonderiliyor..." << std::endl;
                lobbyManager->toggleReady(true);
                readyCommandSent = true;
            }
            else if (readyCommandSent) {
                std::cout << "Zaten hazir durumdasiniz." << std::endl;
            }
            else {
                std::cout << "Gecersiz komut. Hazir olmak icin 'h' yazin." << std::endl;
            }
        }
        });
    // --- Giri� ��leme Thread'i Sonu ---


    // Ana D�ng�
    auto startTime = std::chrono::steady_clock::now();
    while (g_isRunning) {
        netManager.update(0.016f);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        // Discovery'yi sadece ba�lanana kadar g�nder (1 saniyede bir)
        if (!g_isConnected && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() % 1000 < 16) {
            netManager.discovery()->sendDiscoveryRequest();
        }
    }

    // Uygulama kapan�rken giri� thread'ini sonland�rmak i�in
    inputThread.detach();

    if (lobbyManager) delete lobbyManager;
    std::cout << "\nIstemci testi bitti." << std::endl;
}

// --- Ana Fonksiyon ---
int main() {
    // ... (T�rk�e karakter d�zeltmesi opsiyonel) ...

    NetworkManager netManager;
    netManager.setLogger([](const std::string& msg) { /* std::cout << "[LOG] " << msg << std::endl; */ });

    std::cout << "Test modunu secin:\n1: Sunucu\n2: Istemci\nSeciminiz: ";
    int choice;
    if (!(std::cin >> choice)) {
        std::cerr << "Gecersiz girdi. Program sonlandiriliyor." << std::endl;
        return 1;
    }

    if (choice == 1) {
        runServer(netManager, GAME_PORT, DISCOVERY_PORT);
    }
    else if (choice == 2) {
        std::string name;
        std::cout << "Istemci Adinizi Girin: ";
        std::cin >> name;
        runClient(netManager, DISCOVERY_PORT, name);
    }
    else {
        std::cerr << "Gecersiz secim." << std::endl;
    }

    return 0;
}