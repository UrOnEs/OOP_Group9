// NetServer.cpp

#include "Network/NetServer.h"
#include <iostream>
#include <sstream>
#include "Network/NetworkManager.h"
#include "Network/NetServer.h"  // E0833 ve C2027 hatalar�n� ��zer
#include "Network/NetClient.h"

// --- Kurucu ve Y�k�c� ---

NetServer::NetServer()
    : m_port(0), m_nextClientId(2) // Client ID'ler 1'den ba�las�n
{
    // Soketi engellemeyen (non-blocking) moda ayarl�yoruz.
    m_socket.setBlocking(false);
}

NetServer::~NetServer() {
    stop();
}

// --- A� Kontrol� ---

bool NetServer::start(unsigned short port) {
    if (m_socket.bind(port) != sf::Socket::Done) {
        std::cerr << "Hata: Sunucu soketi port " << port << " a ba�lanamad�." << std::endl;
        return false;
    }

    m_port = port;
    std::cout << "Sunucu ba�ar�yla ba�lat�ld� ve port " << port << " dinliyor (UDP). Ba�lang�� ID: " << m_nextClientId << std::endl;
    return true;
}

void NetServer::stop() {
    if (m_port != 0) {
        m_socket.unbind();
        m_port = 0;
        m_connections.clear();
        m_endpointToId.clear();
        std::cout << "Sunucu durduruldu." << std::endl;
    }
}

// --- Ana G�ncelleme D�ng�s� ---

void NetServer::update() {
    sf::Packet receivedPacket;
    sf::IpAddress senderAddress;
    unsigned short senderPort;

    // Engellemeyen modda, bu d�ng� t�m mevcut paketleri bir kerede al�r.
    while (m_socket.receive(receivedPacket, senderAddress, senderPort) == sf::Socket::Done) {

        // 1. Gelen adrese kar��l�k gelen Client ID'yi bul veya yeni bir tane olu�tur/ata.
        uint64_t clientId = ensureConnectionAndGetId(senderAddress, senderPort);

        // 2. Paketi i�le ve callback fonksiyonunu �a��r.
        handleIncomingPacket(receivedPacket, senderAddress, senderPort);

        // �NEML�: SFML, receive() sonras�nda paketin i�eri�ini temizlemez. 
        // D�ng�n�n bir sonraki iterasyonunda yeni paketi almadan �nce paketin yeniden kullan�ma haz�r olmas� gerekir.
        receivedPacket.clear();
    }
}

// --- Client ID Y�netimi ve Ba�lant� Ekleme ---

uint64_t NetServer::ensureConnectionAndGetId(const sf::IpAddress& address, unsigned short port) {
    // 1. Endpoint key olu�tur
    std::string endpointKey = address.toString() + ":" + std::to_string(port);

    // 2. ID'yi map'te ara
    auto id_it = m_endpointToId.find(endpointKey);

    if (id_it == m_endpointToId.end()) {
        // --- Yeni �stemci ---

        uint64_t newId = m_nextClientId++;
        std::cout << "Yeni istemci ba�land�: " << endpointKey << " -> ID: " << newId << std::endl;

        // a) Endpoint'ten ID'ye e�leme
        m_endpointToId[endpointKey] = newId;

        // b) Yeni Connection nesnesini olu�tur
        auto newConn = std::make_unique<Connection>(address, port);

        // c) ID'den Connection'a e�leme
        m_connections[newId] = std::move(newConn);

        // d) Ba�lant� callback'ini �a��r
        if (m_onClientConnectedCallback) {
            m_onClientConnectedCallback(newId);
        }

        return newId;

    }
    else {
        // --- Mevcut �stemci ---
        return id_it->second;
    }
}

// --- Gelen Paket ��leme ---

void NetServer::handleIncomingPacket(sf::Packet& packet, const sf::IpAddress& senderAddress, unsigned short senderPort) {
    // 1. Client ID'yi bul
    std::string endpointKey = senderAddress.toString() + ":" + std::to_string(senderPort);
    uint64_t clientId = m_endpointToId[endpointKey]; // Zaten mevcut oldu�unu biliyoruz

    // 2. E�er callback ayarl�ysa, paketi ilet
    if (m_onPacketCallback) {
        // Paketi callback'e g�nder
        m_onPacketCallback(clientId, packet);
    }
    // Not: Callback fonksiyonu i�inde sf::Packet'ten veriler okunmal�d�r.
    // �rn: sf::Int32 commandType; if (packet >> commandType) { ... }
}

// --- Veri G�nderme ---

bool NetServer::sendTo(uint64_t clientId, sf::Packet& pkt) {
    auto it = m_connections.find(clientId);
    if (it != m_connections.end()) {
        // �lgili Connection nesnesi �zerinden, NetServer'�n tek soketini kullanarak g�nderim yap.
        sf::Socket::Status status = it->second->send(m_socket, pkt);
        return status == sf::Socket::Done;
    }
    // Ba�lant� (ID) bulunamad�.
    return false;
}

bool NetServer::sendTo(const std::string& endpoint, sf::Packet& pkt) {
    // �nce Endpoint'ten ID'yi bul
    auto id_it = m_endpointToId.find(endpoint);
    if (id_it != m_endpointToId.end()) {
        return sendTo(id_it->second, pkt);
    }
    return false;
}



void NetServer::sendToAll(sf::Packet& pkt) { // void olarak kalabilir, ��nk� sendToAll(broadcast) genellikle void d�ner

    // T�m ba�l� Connection'lara yay�n yap.
    // Geleneksel iterat�r kullanarak hatay� ��z�yoruz:
    for (auto const& pair : m_connections) {
        // 'pair' �imdi std::pair<const uint64_t, std::unique_ptr<Connection>> t�r�ndedir.

        Connection* connPtr = pair.second.get(); // unique_ptr i�indeki ham pointer'� al

        if (connPtr) {
            connPtr->send(m_socket, pkt);
        }
    }
}



void NetServer::broadcast(sf::Packet& pkt) {
    // T�m ba�l� Connection'lara yay�n yap.
    // Geleneksel iterat�r kullanarak hatay� ��z�yoruz:
    for (auto const& pair : m_connections) {
        // 'pair' �imdi std::pair<const uint64_t, std::unique_ptr<Connection>> t�r�ndedir.

        // uint64_t clientId = pair.first; // clientId'ye eri�im (iste�e ba�l�)
        Connection* connPtr = pair.second.get(); // unique_ptr i�indeki ham pointer'� al

        if (connPtr) {
            connPtr->send(m_socket, pkt);
        }
    }
}

// --- Callback Ayarlay�c�lar ---

void NetServer::setOnPacket(OnPacketFn cb) {
    m_onPacketCallback = std::move(cb);
}

void NetServer::setOnClientConnected(std::function<void(uint64_t)> cb) {
    m_onClientConnectedCallback = std::move(cb);
}

// Not: UDP'de disconncted callback'i, zaman a��m� (timeout) mant��� gerektirir.
// �u anki minimal prototipte bu mant�k implemente edilmemi�tir, 
// ancak burada bir setter oldu�unu bilmek yeterlidir.
void NetServer::setOnClientDisconnected(std::function<void(uint64_t)> cb) {
    m_onClientDisconnectedCallback = std::move(cb);
}