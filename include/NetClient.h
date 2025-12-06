#pragma once
// NetClient.h - SFML UDP/Packet

#pragma once

#include <SFML/Network.hpp> // sf::UdpSocket ve sf::Packet i�in
#include <functional>
#include <string>

// Not: Eski yap�n�zdaki 'Packet.h' yerine, sf::Packet kullanaca��z.

class NetClient {
public:
    // Callback fonksiyonu sf::Packet almal�d�r.
    using OnPacketFn = std::function<void(sf::Packet& packet)>;

    NetClient();
    ~NetClient();

    // Ba�lant� adresi ve portu kaydedilir ve soket ba�lan�r (bind).
    bool connect(const std::string& addr, unsigned short port);
    void disconnect();

    // A� trafi�ini i�ler (gelen paketleri al�r).
    void update(float dt);

    // Paketi sunucuya g�nderir.
    bool send(sf::Packet& pkt);

    // --- Callback Ayarlay�c�lar ---
    void setOnPacket(OnPacketFn cb);
    // UDP'de connect callback'i, soket ba�ar�yla ba�land���nda (bind) �a�r�l�r.
    void setOnConnected(std::function<void()> cb);
    // Sunucudan belirli bir s�re (timeout) yan�t al�namazsa �a�r�labilir (implementasyon detayd�r).
    void setOnDisconnected(std::function<void()> cb);

    // Ba�lant� durumunu kontrol eder (UDP i�in: Soket ba�l� m� ve sunucu adresi kay�tl� m�?).
    bool isConnected() const;

private:
    sf::UdpSocket m_socket;
    sf::IpAddress m_serverAddress;
    unsigned short m_serverPort;
    bool m_connected; // Sunucu adresinin ba�ar�yla kaydedilip edilmedi�i

    // --- Callback Fonksiyonlar� ---
    OnPacketFn m_onPacketCallback;
    std::function<void()> m_onConnectedCallback;
    std::function<void()> m_onDisconnectedCallback;

    // Gelen bir paketi i�leyen metot
    void handleIncomingPacket(sf::Packet& packet);
};