#pragma once
// NetServer.h - SFML UDP/Packet

#pragma once

#include "Connection.h" // Connection, IP ve Port tutar
#include <SFML/Network.hpp> // sf::UdpSocket ve sf::Packet i�in
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>
#include <string> // endpoint key i�in


class NetServer {
public:
    // �stemci ID'si yerine (UDP'de IP:Port adresi daha �nemlidir, ancak eski yap�y� koruyal�m)
    // Packet yerine sf::Packet kullan�yoruz.
    using OnPacketFn = std::function<void(uint64_t clientId, sf::Packet& packet)>;

    NetServer();
    ~NetServer();

    bool start(unsigned short port);
    void stop();

    // dt parametresini kald�r�p sadece update() b�rakt�k, UDP'de zamana ba�l� i�lem azd�r.
    void update();

    // sf::Packet ile yay�n yapar.
    void broadcast(sf::Packet& pkt);

    // Belirli bir istemciye IP:Port �zerinden g�nderim yapar.
    bool sendTo(uint64_t clientId, sf::Packet& pkt);

    // (Opsiyonel: IP:Port string'ini kullanarak g�ndermek daha pratik olabilir.)
    bool sendTo(const std::string& endpoint, sf::Packet& pkt);

    bool sendToReliable(uint64_t clientId, sf::Packet& pkt);

    void sendToAll(sf::Packet& pkt);

    void sendToAllReliable(sf::Packet& pkt);

    // NetServer sınıfının public kısmına:
    void sendToAllExcept(uint64_t excludedClientId, sf::Packet& pkt);


    // --- Callback Ayarlay�c�lar ---
    void setOnPacket(OnPacketFn cb);
    // UDP ba�lant�s�zd�r, ancak bir istemci ilk mesaj�n� g�nderdi�inde 'ba�lanm��' kabul ederiz.
    void setOnClientConnected(std::function<void(uint64_t)> cb);
    // Bir istemciden uzun s�re mesaj gelmedi�inde 'ba�lant� kesilmi�' kabul edilebilir (Keep-Alive mant���).
    void setOnClientDisconnected(std::function<void(uint64_t)> cb);

private:
    sf::UdpSocket m_socket;
    unsigned short m_port;

    // Key: uint64_t Client ID (Oyun sunucusu seviyesinde atanm�� kimlik)
    // Value: Connection nesnesine pointer.
    std::map<uint64_t, std::unique_ptr<Connection>> m_connections;

    // Key: Endpoint (IP:Port string'i), Value: uint64_t Client ID
    // H�zl� arama ve Client ID atamas� i�in gereklidir.
    std::map<std::string, uint64_t> m_endpointToId;

    // Basit bir ID atay�c�s�
    uint64_t m_nextClientId;

    // --- Fonksiyon Pointerlar� ---
    OnPacketFn m_onPacketCallback;
    std::function<void(uint64_t)> m_onClientConnectedCallback;
    std::function<void(uint64_t)> m_onClientDisconnectedCallback;

    // Gelen bir paketi i�leyen metot
    void handleIncomingPacket(sf::Packet& packet, const sf::IpAddress& senderAddress, unsigned short senderPort);

    // Gelen adresi ba�lant� listemize ekler, ID atar veya mevcut olan� d�nd�r�r.
    uint64_t ensureConnectionAndGetId(const sf::IpAddress& address, unsigned short port);
};