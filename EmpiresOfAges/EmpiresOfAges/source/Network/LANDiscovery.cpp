#include "Network/LANDiscovery.h"
#include <iostream>

LANDiscovery::LANDiscovery()
    : m_broadcastPort(0), m_gamePort(0)
{
    m_socket.setBlocking(false);
}

LANDiscovery::~LANDiscovery() {
    stop();
}

void LANDiscovery::stop() {
    m_socket.unbind();
}

// -----------------------------------------------------
// SERVER SIDE
// -----------------------------------------------------
bool LANDiscovery::startServer(unsigned short listenGamePort, unsigned short announcePort, const std::string& serverName) {
    m_gamePort = listenGamePort;
    m_broadcastPort = announcePort;
    m_serverName = serverName;

    if (m_socket.bind(m_broadcastPort) != sf::Socket::Done) {
        std::cerr << "[LANDiscovery] ERROR: Server cannot bind to discovery port " << m_broadcastPort << "\n";
        return false;
    }
    return true;
}

// -----------------------------------------------------
// CLIENT SIDE
// -----------------------------------------------------
bool LANDiscovery::startClient(unsigned short broadcastPort) {
    m_broadcastPort = broadcastPort;
    m_discoveryActive = true;
    return true;
}

void LANDiscovery::sendDiscoveryRequest() {
    if (m_broadcastPort == 0) return;

    sf::Packet pkt;
    pkt << static_cast<sf::Int32>(Command::Request);
    m_socket.send(pkt, sf::IpAddress::Broadcast, m_broadcastPort);
}

// -----------------------------------------------------
// UPDATE LOOP
// -----------------------------------------------------
void LANDiscovery::update() {
    sf::Packet pkt;
    sf::IpAddress sender;
    unsigned short port;

    while (m_socket.receive(pkt, sender, port) == sf::Socket::Done) {
        handleIncomingPacket(pkt, sender, port);
    }
}

void LANDiscovery::handleIncomingPacket(sf::Packet& pkt, const sf::IpAddress& senderAddr, unsigned short senderPort) {
    sf::Int32 cmdInt;

    if (!(pkt >> cmdInt))
        return;

    Command cmd = static_cast<Command>(cmdInt);

    // REQUEST -> SERVER SENDS RESPONSE
    if (cmd == Command::Request) {
        if (m_serverName.empty()) return; // Ignore if we are a client

        sf::Packet response;
        response << static_cast<sf::Int32>(Command::Response)
            << m_serverName
            << m_gamePort;

        m_socket.send(response, senderAddr, senderPort);
    }

    // RESPONSE -> CLIENT FINDS SERVER
    if (cmd == Command::Response) {
        std::string name;
        unsigned short gamePort;

        if (pkt >> name >> gamePort) {
            ServerInfo info{ senderAddr, gamePort, name };
            if (m_onServerFoundCallback)
                m_onServerFoundCallback(info);
        }
    }
}

void LANDiscovery::setOnServerFound(OnServerFoundFn cb) {
    m_onServerFoundCallback = std::move(cb);
}