#pragma once
#pragma once
#include <SFML/Network.hpp>
#include <functional>
#include <vector>
#include <string>


class LANDiscovery {
public:
    enum class Command {
        Request = 1,
        Response = 2
    };

    struct ServerInfo {
        sf::IpAddress address;
        unsigned short port;
        std::string name;
    };

    using OnServerFoundFn = std::function<void(const ServerInfo&)>;

    LANDiscovery();
    ~LANDiscovery();

    void stop();

    bool startServer(unsigned short listenGamePort, unsigned short announcePort, const std::string& serverName);
    bool startClient(unsigned short broadcastPort);

    void sendDiscoveryRequest();
    bool m_discoveryActive = false;
    void update();

    void setOnServerFound(OnServerFoundFn cb);

private:
    void handleIncomingPacket(sf::Packet& pkt, const sf::IpAddress& senderAddr, unsigned short senderPort);

private:
    sf::UdpSocket m_socket;
    unsigned short m_broadcastPort;
    unsigned short m_gamePort;
    std::string m_serverName;
    OnServerFoundFn m_onServerFoundCallback;
};
