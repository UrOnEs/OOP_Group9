#pragma once
// Connection.h - SFML UDP/Packet Versiyonu

#pragma once

#include <SFML/Network.hpp>
#include <string>

// Eski enum'u koruyoruz, ancak UDP'de ConnectionState'in kullan�m� farkl� olacakt�r.
enum class ConnectionState { Disconnected, Connected };
// UDP s�rekli ba�lant� kurmad��� i�in 'Connecting' ve 'Error' durumlar�n� bu seviyede ele alm�yoruz.

class Connection {
public:
    // Sadece IP adresi ve portu tutan kurucu
    Connection(const sf::IpAddress& address, unsigned short port);

    // SFML'in paketlerini kullanaca��m�z i�in Packet s�n�f�n� kullanabiliriz.
    // Ancak UDP soketi bu s�n�fa ait olmad���ndan, send metodu soketi parametre olarak almal�.
    sf::Socket::Status send(sf::UdpSocket& socket, sf::Packet& packet) const;


    // UDP'de s�rekli bir receive d�ng�s� yoktur. Sunucu/�stemci s�n�f� soketten s�rekli veri al�r.

    // UDP'de bir uzak adres her zaman eri�ilebilir varsay�l�r.
    ConnectionState state() const { return ConnectionState::Connected; }

    // Uzak Adres ve Port bilgisini d�nd�r�r.
    std::string endpoint() const;

    // --- Eri�imciler ---
    sf::IpAddress getAddress() const { return m_address; }
    unsigned short getPort() const { return m_port; }

private:
    sf::IpAddress m_address;
    unsigned short m_port;

    // �statistikler i�in ekleyebiliriz (opsiyonel)
    uint64_t m_bytesSent = 0;
    // uint64_t m_bytesReceived = 0; // Bu istatistik sunucu/istemci seviyesinde tutulur
};