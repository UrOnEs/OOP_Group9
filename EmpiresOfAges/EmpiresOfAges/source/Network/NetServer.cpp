#include "Network/NetServer.h"
#include <iostream>
#include <sstream>
#include "Network/NetworkManager.h"
#include "Network/NetClient.h"

NetServer::NetServer()
    : m_port(0), m_nextClientId(2) // Client IDs start from 2 (1 is usually host)
{
    m_socket.setBlocking(false);
}

NetServer::~NetServer() {
    stop();
}

bool NetServer::start(unsigned short port) {
    if (m_socket.bind(port) != sf::Socket::Done) {
        std::cerr << "Error: Server socket could not bind to port " << port << std::endl;
        return false;
    }

    m_port = port;
    return true;
}

void NetServer::stop() {
    if (m_port != 0) {
        m_socket.unbind();
        m_port = 0;
        m_connections.clear();
        m_endpointToId.clear();
    }
}

void NetServer::update() {
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;

    while (m_socket.receive(packet, sender, port) == sf::Socket::Done) {
        uint64_t clientId = ensureConnectionAndGetId(sender, port);

        sf::Uint8 typeRaw;
        if (packet >> typeRaw) {
            PacketType type = static_cast<PacketType>(typeRaw);

            // --- ACK CHECK ---
            if (type == PacketType::ACK) {
                sf::Uint32 seq;
                if (packet >> seq) {
                    if (m_connections.find(clientId) != m_connections.end()) {
                        m_connections[clientId]->processACK(seq);
                    }
                }
                packet.clear();
                continue;
            }

            // --- RELIABLE CHECK ---
            if (type == PacketType::Reliable) {
                sf::Uint32 seq;
                if (packet >> seq) {
                    // Send ACK Response
                    sf::Packet ackPkt;
                    ackPkt << static_cast<sf::Uint8>(PacketType::ACK) << seq;
                    m_connections[clientId]->send(m_socket, ackPkt);
                }
                else {
                    packet.clear(); continue;
                }
            }

            // --- PAYLOAD ---
            if (m_onPacketCallback) {
                m_onPacketCallback(clientId, packet);
            }
        }
        packet.clear();
    }

    // Resend pending packets for all connections
    for (auto& pair : m_connections) {
        pair.second->resendMissingPackets(m_socket);
    }
}

uint64_t NetServer::ensureConnectionAndGetId(const sf::IpAddress& address, unsigned short port) {
    std::string endpointKey = address.toString() + ":" + std::to_string(port);
    auto id_it = m_endpointToId.find(endpointKey);

    if (id_it == m_endpointToId.end()) {
        // --- New Client ---
        uint64_t newId = m_nextClientId++;

        m_endpointToId[endpointKey] = newId;
        auto newConn = std::make_unique<Connection>(address, port);
        m_connections[newId] = std::move(newConn);

        if (m_onClientConnectedCallback) {
            m_onClientConnectedCallback(newId);
        }

        return newId;
    }
    else {
        // --- Existing Client ---
        return id_it->second;
    }
}

void NetServer::handleIncomingPacket(sf::Packet& packet, const sf::IpAddress& senderAddress, unsigned short senderPort) {
    std::string endpointKey = senderAddress.toString() + ":" + std::to_string(senderPort);
    uint64_t clientId = m_endpointToId[endpointKey];

    if (m_onPacketCallback) {
        m_onPacketCallback(clientId, packet);
    }
}

bool NetServer::sendTo(uint64_t clientId, sf::Packet& pkt) {
    auto it = m_connections.find(clientId);
    if (it != m_connections.end()) {
        sf::Packet finalPacket;
        finalPacket << static_cast<sf::Uint8>(PacketType::Unreliable);
        finalPacket.append(pkt.getData(), pkt.getDataSize());

        sf::Socket::Status status = it->second->send(m_socket, finalPacket);
        return status == sf::Socket::Done;
    }
    return false;
}

bool NetServer::sendTo(const std::string& endpoint, sf::Packet& pkt) {
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

            sf::Packet finalPkt;
            finalPkt << (sf::Uint8)PacketType::Reliable << seq;
            finalPkt.append(pkt.getData(), pkt.getDataSize());

            conn->send(m_socket, finalPkt);
            conn->addPendingPacket(seq, finalPkt);
            return true;
        }
    }
    return false;
}

void NetServer::sendToAll(sf::Packet& pkt) {
    broadcast(pkt);
}

void NetServer::sendToAllReliable(sf::Packet& pkt) {
    for (auto& pair : m_connections) {
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

void NetServer::sendToAllExcept(uint64_t excludedClientId, sf::Packet& pkt) {
    for (auto& pair : m_connections) {
        if (pair.first != excludedClientId) {
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

void NetServer::setOnPacket(OnPacketFn cb) {
    m_onPacketCallback = std::move(cb);
}

void NetServer::setOnClientConnected(std::function<void(uint64_t)> cb) {
    m_onClientConnectedCallback = std::move(cb);
}

void NetServer::setOnClientDisconnected(std::function<void(uint64_t)> cb) {
    m_onClientDisconnectedCallback = std::move(cb);
}