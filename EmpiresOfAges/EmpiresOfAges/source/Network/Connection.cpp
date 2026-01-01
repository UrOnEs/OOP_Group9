// Connection.cpp - SFML UDP/Packet

#include "Network/Connection.h"
#include <iostream>

// --- Kurucu ---

Connection::Connection(const sf::IpAddress& address, unsigned short port)
    : m_address(address), m_port(port) {}

// --- A� ��lemleri Implementasyonu ---

sf::Socket::Status Connection::send(sf::UdpSocket& socket, sf::Packet& packet) const {
    // SFML'in UdpSocket'�, paketi belirtilen uzak adrese ve porta g�nderir.
    sf::Socket::Status status = socket.send(packet, m_address, m_port);

    // �statistik g�ncellemesi
    if (status == sf::Socket::Done) {
        // sf::Packet'�n yakla��k boyutunu hesaplamak zor oldu�u i�in, 
        // pratiklik ad�na sadece g�nderim ba�ar�l�ysa art�rabiliriz.
        // Daha do�ru bir say�m i�in sf::Packet'�n verilerini okumak gerekir, �imdilik atl�yoruz.
        // m_bytesSent += packet.getDataSize(); 
    }

    return status;
}

std::string Connection::endpoint() const {
    // �rnek: "192.168.1.1:54000"
    return m_address.toString() + ":" + std::to_string(m_port);
}

void Connection::addPendingPacket(uint32_t seq, sf::Packet pkt) {
    m_pendingPackets[seq] = { pkt, sf::Clock(), seq };
}

void Connection::processACK(uint32_t seq) {
    m_pendingPackets.erase(seq); // Onay geldi, listeden çıkar
}

void Connection::resendMissingPackets(sf::UdpSocket& socket) {
    for (auto& pair : m_pendingPackets) {
        // Süreyi 50ms yapmıştık, kontrol edin
        if (pair.second.timer.getElapsedTime().asMilliseconds() > 50) {

            // --- LOG EKLEMESİ ---
            std::cout << "[Network] Paket (Seq: " << pair.second.sequence
                << ") tekrar gonderiliyor -> " << m_address.toString() << "\n";
            // --------------------

            socket.send(pair.second.packet, m_address, m_port);
            pair.second.timer.restart();
        }
    }
}