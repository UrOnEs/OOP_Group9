#pragma once
#include "Connection.h" 
#include <SFML/Network.hpp> 
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>
#include <string> 

/**
 * @brief Manages the server-side network logic using UDP.
 * Handles multiple client connections, packet broadcasting, and reliability.
 */
class NetServer {
public:
    using OnPacketFn = std::function<void(uint64_t clientId, sf::Packet& packet)>;

    NetServer();
    ~NetServer();

    bool start(unsigned short port);
    void stop();

    void update();

    void broadcast(sf::Packet& pkt);
    bool sendTo(uint64_t clientId, sf::Packet& pkt);
    bool sendTo(const std::string& endpoint, sf::Packet& pkt);
    bool sendToReliable(uint64_t clientId, sf::Packet& pkt);
    void sendToAll(sf::Packet& pkt);
    void sendToAllReliable(sf::Packet& pkt);
    void sendToAllExcept(uint64_t excludedClientId, sf::Packet& pkt);

    // --- Callbacks ---
    void setOnPacket(OnPacketFn cb);
    void setOnClientConnected(std::function<void(uint64_t)> cb);
    void setOnClientDisconnected(std::function<void(uint64_t)> cb);

private:
    sf::UdpSocket m_socket;
    unsigned short m_port;

    // Key: Client ID, Value: Connection object
    std::map<uint64_t, std::unique_ptr<Connection>> m_connections;

    // Key: Endpoint (IP:Port), Value: Client ID
    std::map<std::string, uint64_t> m_endpointToId;

    uint64_t m_nextClientId;

    OnPacketFn m_onPacketCallback;
    std::function<void(uint64_t)> m_onClientConnectedCallback;
    std::function<void(uint64_t)> m_onClientDisconnectedCallback;

    void handleIncomingPacket(sf::Packet& packet, const sf::IpAddress& senderAddress, unsigned short senderPort);
    uint64_t ensureConnectionAndGetId(const sf::IpAddress& address, unsigned short port);
};