#pragma once
#include <SFML/Network.hpp>
#include <functional>
#include <vector>
#include <string>

/**
 * @brief Handles LAN discovery using UDP broadcasting.
 * Allows clients to find servers running on the local network.
 */
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

    /**
     * @brief Starts the discovery server to listen for broadcast requests.
     * @param listenGamePort The actual game port to report to clients.
     * @param announcePort The port to listen for discovery broadcasts (e.g. 50000).
     * @param serverName The visible name of the server.
     */
    bool startServer(unsigned short listenGamePort, unsigned short announcePort, const std::string& serverName);

    /**
     * @brief Starts the discovery client to search for servers.
     * @param broadcastPort The port to send broadcast requests to.
     */
    bool startClient(unsigned short broadcastPort);

    /**
     * @brief Sends a broadcast packet requesting server info.
     */
    void sendDiscoveryRequest();

    void update();
    void setOnServerFound(OnServerFoundFn cb);

    bool m_discoveryActive = false;

private:
    void handleIncomingPacket(sf::Packet& pkt, const sf::IpAddress& senderAddr, unsigned short senderPort);

    sf::UdpSocket m_socket;
    unsigned short m_broadcastPort;
    unsigned short m_gamePort;
    std::string m_serverName;
    OnServerFoundFn m_onServerFoundCallback;
};