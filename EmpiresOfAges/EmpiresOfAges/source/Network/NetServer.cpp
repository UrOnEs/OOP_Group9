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

// source/Network/NetServer.cpp - update() fonksiyonunu güncelle
void NetServer::update() {
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;

    while (m_socket.receive(packet, sender, port) == sf::Socket::Done) {
        // İstemci ID'sini bul veya oluştur
        uint64_t clientId = ensureConnectionAndGetId(sender, port);

        // 1. Header Oku
        sf::Uint8 typeRaw;
        if (packet >> typeRaw) {
            PacketType type = static_cast<PacketType>(typeRaw);

            // --- ACK KONTROLÜ ---
            if (type == PacketType::ACK) {
                sf::Uint32 seq;
                if (packet >> seq) {
                    if (m_connections.find(clientId) != m_connections.end()) {
                        m_connections[clientId]->processACK(seq);
                    }
                }
                // ACK işlendi, sonraki pakete geç
                packet.clear();
                continue;
            }

            // --- RELIABLE KONTROLÜ ---
            if (type == PacketType::Reliable) {
                sf::Uint32 seq;
                if (packet >> seq) {
                    // ACK Cevabı Gönder
                    sf::Packet ackPkt;
                    ackPkt << static_cast<sf::Uint8>(PacketType::ACK) << seq;
                    // Not: ACK paketlerini reliable sarmaya gerek yok, direkt gönder
                    m_connections[clientId]->send(m_socket, ackPkt);
                }
                else {
                    packet.clear(); continue; // Hatalı paket
                }
            }

            // --- UNRELIABLE ---
            // Unreliable ise sadece type okundu, veri hazır.

            // 2. Paketin geri kalanını (Payload) oyun mantığına ver
            // handleIncomingPacket fonksiyonu ID bulup callback'i çağırır.
            // Ancak handleIncomingPacket tekrar ensureConnection yapmasın diye
            // direkt callback'i çağırmak daha performanslıdır ama yapıyı bozmayalım:

            // Mevcut yapıda handleIncomingPacket fonksiyonunu modifiye etmeden kullanmak için:
            // "handleIncomingPacket" fonksiyonu "endpointToId" kullanıyor.
            // Paket imleci şu an verinin (Command) başında.

            if (m_onPacketCallback) {
                m_onPacketCallback(clientId, packet);
            }
        }

        packet.clear();
    }

    // Bekleyen paketleri tekrar gönder
    for (auto& pair : m_connections) {
        pair.second->resendMissingPackets(m_socket);
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

        // Sarmalayıcı paket
        sf::Packet finalPacket;
        finalPacket << static_cast<sf::Uint8>(PacketType::Unreliable);
        finalPacket.append(pkt.getData(), pkt.getDataSize());

        sf::Socket::Status status = it->second->send(m_socket, finalPacket);
        return status == sf::Socket::Done;
    }
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

bool NetServer::sendToReliable(uint64_t clientId, sf::Packet& pkt) {
    auto it = m_connections.find(clientId);
    if (it != m_connections.end()) {
        Connection* conn = it->second.get();
        if (conn) {
            uint32_t seq = conn->getNextSequence();

            // Reliable Başlık Ekle
            sf::Packet finalPkt;
            finalPkt << (sf::Uint8)PacketType::Reliable << seq;
            finalPkt.append(pkt.getData(), pkt.getDataSize());

            // Gönder ve Takip Listesine Ekle
            conn->send(m_socket, finalPkt);
            conn->addPendingPacket(seq, finalPkt);
            return true;
        }
    }
    return false;
}



void NetServer::sendToAll(sf::Packet& pkt) { // void olarak kalabilir, ��nk� sendToAll(broadcast) genellikle void d�ner
    broadcast(pkt);
}

void NetServer::sendToAllReliable(sf::Packet& pkt) {
    for (auto& pair : m_connections) {
        Connection* conn = pair.second.get();
        if (conn) {
            // 1. Sequence numarasını al
            uint32_t seq = conn->getNextSequence();

            // 2. Reliable Header Ekle
            sf::Packet finalPkt;
            finalPkt << (sf::Uint8)PacketType::Reliable << seq;
            finalPkt.append(pkt.getData(), pkt.getDataSize());

            // 3. Gönder ve Onay Listesine Ekle
            conn->send(m_socket, finalPkt);
            conn->addPendingPacket(seq, finalPkt);
        }
    }
}

void NetServer::sendToAllExcept(uint64_t excludedClientId, sf::Packet& pkt) {
    for (auto& pair : m_connections) {
        if (pair.first != excludedClientId) { // Gönderen hariç diğerlerine yolla
            Connection* conn = pair.second.get();
            if (conn) {
                uint32_t seq = conn->getNextSequence();
                sf::Packet finalPkt;
                finalPkt << (sf::Uint8)PacketType::Reliable << seq;
                finalPkt.append(pkt.getData(), pkt.getDataSize());
                conn->send(m_socket, finalPkt);
                conn->addPendingPacket(seq, finalPkt);
            }
        }
    }
}


void NetServer::broadcast(sf::Packet& pkt) {
    // Sarmalayıcı paket (Sadece bir kez oluşturuyoruz)
    sf::Packet finalPacket;
    finalPacket << static_cast<sf::Uint8>(PacketType::Unreliable);
    finalPacket.append(pkt.getData(), pkt.getDataSize());

    for (auto const& pair : m_connections) {
        Connection* connPtr = pair.second.get();
        if (connPtr) {
            connPtr->send(m_socket, finalPacket);
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