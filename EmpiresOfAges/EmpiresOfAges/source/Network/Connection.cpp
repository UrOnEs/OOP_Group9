#include "Network/Connection.h"
#include <iostream>

Connection::Connection(const sf::IpAddress& address, unsigned short port)
    : m_address(address), m_port(port) {}

sf::Socket::Status Connection::send(sf::UdpSocket& socket, sf::Packet& packet) const {
    // SFML UdpSocket sends the packet to the stored address/port.
    sf::Socket::Status status = socket.send(packet, m_address, m_port);

    if (status == sf::Socket::Done) {
        // Stats could be updated here.
        // m_bytesSent += packet.getDataSize(); 
    }

    return status;
}

std::string Connection::endpoint() const {
    return m_address.toString() + ":" + std::to_string(m_port);
}

void Connection::addPendingPacket(uint32_t seq, sf::Packet pkt) {
    m_pendingPackets[seq] = { pkt, sf::Clock(), seq };
}

void Connection::processACK(uint32_t seq) {
    m_pendingPackets.erase(seq); // ACK received, remove from pending.
}

void Connection::resendMissingPackets(sf::UdpSocket& socket) {
    for (auto& pair : m_pendingPackets) {
        // Retry timeout check (50ms)
        if (pair.second.timer.getElapsedTime().asMilliseconds() > 50) {
            // Retrying send... (Debug logs removed for cleaner output)
            socket.send(pair.second.packet, m_address, m_port);
            pair.second.timer.restart();
        }
    }
}