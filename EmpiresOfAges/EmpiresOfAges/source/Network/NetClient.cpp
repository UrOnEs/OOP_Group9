#include "Network/NetClient.h"
#include <iostream>
#include <thread>
#include <chrono>

NetClient::NetClient()
    : m_serverPort(0), m_connected(false)
{
    m_socket.setBlocking(false);
}

NetClient::~NetClient() {
    disconnect();
}

bool NetClient::connect(const std::string& addr, unsigned short port) {
    if (isConnected()) {
        disconnect();
    }

    m_serverAddress = sf::IpAddress(addr);
    m_serverPort = port;

    // UDP client must bind to a port to receive responses. 
    // AnyPort (0) lets the OS choose.
    if (m_socket.bind(sf::Socket::AnyPort) != sf::Socket::Done) {
        std::cerr << "Error: Client socket could not bind to any port." << std::endl;
        m_serverPort = 0;
        return false;
    }

    m_connected = true;

    if (m_onConnectedCallback) {
        m_onConnectedCallback();
    }

    return true;
}

void NetClient::disconnect() {
    if (m_connected) {
        m_socket.unbind();
        m_serverPort = 0;
        m_connected = false;

        if (m_onDisconnectedCallback) {
            m_onDisconnectedCallback();
        }
    }
}

void NetClient::update(float dt) {
    if (!m_connected) return;

    sf::Packet receivedPacket;
    sf::IpAddress senderAddress;
    unsigned short senderPort;

    while (m_socket.receive(receivedPacket, senderAddress, senderPort) == sf::Socket::Done) {
        if (senderPort == m_serverPort) {
            handleIncomingPacket(receivedPacket);
        }
        else {
            std::cerr << "Warning: Packet received from unknown source: "
                << senderAddress.toString() << ":" << senderPort << std::endl;
        }
        receivedPacket.clear();
    }

    // Resend pending reliable packets if timeout reached
    for (auto& pair : m_pendingPackets) {
        if (pair.second.timer.getElapsedTime().asMilliseconds() > 50) {
            m_socket.send(pair.second.packet, m_serverAddress, m_serverPort);
            pair.second.timer.restart();
        }
    }
}

void NetClient::handleIncomingPacket(sf::Packet& packet) {
    sf::Uint8 typeRaw;
    if (!(packet >> typeRaw)) return;

    PacketType type = static_cast<PacketType>(typeRaw);

    // ACK Processing
    if (type == PacketType::ACK) {
        sf::Uint32 confirmedSeq;
        if (packet >> confirmedSeq) {
            m_pendingPackets.erase(confirmedSeq);
        }
        return;
    }

    // Reliable Processing
    if (type == PacketType::Reliable) {
        sf::Uint32 seq;
        if (packet >> seq) {
            // Send ACK back
            sf::Packet ackPkt;
            ackPkt << static_cast<sf::Uint8>(PacketType::ACK) << seq;
            m_socket.send(ackPkt, m_serverAddress, m_serverPort);
        }
        else {
            return;
        }
    }

    // Pass payload to game logic
    if (m_onPacketCallback) {
        m_onPacketCallback(packet);
    }
}

bool NetClient::send(sf::Packet& pkt) {
    if (!m_connected) return false;

    sf::Packet finalPacket;
    finalPacket << static_cast<sf::Uint8>(PacketType::Unreliable);
    finalPacket.append(pkt.getData(), pkt.getDataSize());

    sf::Socket::Status status = m_socket.send(finalPacket, m_serverAddress, m_serverPort);
    return status == sf::Socket::Done;
}

bool NetClient::sendReliable(sf::Packet& pkt) {
    if (!m_connected) return false;

    uint32_t seq = ++m_lastSequenceSent;

    sf::Packet reliablePkt;
    reliablePkt << static_cast<sf::Uint8>(PacketType::Reliable) << seq;
    reliablePkt.append(pkt.getData(), pkt.getDataSize());

    PendingPacket pending;
    pending.packet = reliablePkt;
    pending.sequence = seq;
    pending.timer.restart();
    m_pendingPackets[seq] = pending;

    return m_socket.send(reliablePkt, m_serverAddress, m_serverPort) == sf::Socket::Done;
}

bool NetClient::isConnected() const {
    return m_connected;
}

void NetClient::setOnPacket(OnPacketFn cb) {
    m_onPacketCallback = std::move(cb);
}

void NetClient::setOnConnected(std::function<void()> cb) {
    m_onConnectedCallback = std::move(cb);
}

void NetClient::setOnDisconnected(std::function<void()> cb) {
    m_onDisconnectedCallback = std::move(cb);
}