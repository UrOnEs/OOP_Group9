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
// SERVER
// -----------------------------------------------------
bool LANDiscovery::startServer(unsigned short listenGamePort, unsigned short announcePort, const std::string& serverName) {
    m_gamePort = listenGamePort;
    m_broadcastPort = announcePort;
    m_serverName = serverName;

    // *** BURASI ÇOK KRİTİK *** — discovery portuna bind ol
    if (m_socket.bind(m_broadcastPort) != sf::Socket::Done) {
        std::cout << "[LANDiscovery] ERROR: server cannot bind to port " << m_broadcastPort << "\n";
        return false;
    }

    std::cout << "[LANDiscovery] Server listening on discovery port " << m_broadcastPort
        << ", game port = " << m_gamePort << std::endl;

    return true;
}

// -----------------------------------------------------
// CLIENT
// -----------------------------------------------------
bool LANDiscovery::startClient(unsigned short broadcastPort) {
    m_broadcastPort = broadcastPort;
    m_discoveryActive = true;

    std::cout << "[LANDiscovery] Client started searching on port "
        << broadcastPort << std::endl;
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

    // REQUEST → SERVER CEVAP VERİR
    if (cmd == Command::Request) {
        if (m_serverName.empty())
            return; // Client isek ignore

        std::cout << "[LANDiscovery] Discovery REQUEST received from "
            << senderAddr.toString() << " → sending RESPONSE...\n";

        sf::Packet response;
        response << static_cast<sf::Int32>(Command::Response)
            << m_serverName
            << m_gamePort;

        m_socket.send(response, senderAddr, senderPort);
    }

    // RESPONSE → CLIENT SUNUCUYU BULDU
    if (cmd == Command::Response) {
        std::string name;
        unsigned short gamePort;

        if (pkt >> name >> gamePort) {
            ServerInfo info{ senderAddr, gamePort, name };

            std::cout << "[LANDiscovery] SERVER FOUND: "
                << name << " (" << senderAddr.toString()
                << ":" << gamePort << ")\n";

            if (m_onServerFoundCallback)
                m_onServerFoundCallback(info);
        }
    }
}

void LANDiscovery::setOnServerFound(OnServerFoundFn cb) {
    m_onServerFoundCallback = std::move(cb);
}
