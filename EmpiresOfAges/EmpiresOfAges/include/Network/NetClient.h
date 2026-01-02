#pragma once
#include <SFML/Network.hpp>
#include <functional>
#include <string>
#include <map>
#include "NetCommands.h"

/**
 * @brief Manages the client-side network communication using UDP.
 */
class NetClient {
public:
    using OnPacketFn = std::function<void(sf::Packet& packet)>;

    NetClient();
    ~NetClient();

    /**
     * @brief Connects to a server given an IP and port.
     */
    bool connect(const std::string& addr, unsigned short port);

    /**
     * @brief Disconnects and unbinds the socket.
     */
    void disconnect();

    /**
     * @brief Polls incoming packets. Call this every frame.
     */
    void update(float dt);

    /**
     * @brief Sends an unreliable packet (fire-and-forget).
     */
    bool send(sf::Packet& pkt);

    /**
     * @brief Sends a reliable packet ensuring delivery via ACKs.
     */
    bool sendReliable(sf::Packet& pkt);

    // --- Callbacks ---
    void setOnPacket(OnPacketFn cb);
    void setOnConnected(std::function<void()> cb);
    void setOnDisconnected(std::function<void()> cb);

    bool isConnected() const;

private:
    sf::UdpSocket m_socket;
    sf::IpAddress m_serverAddress;
    unsigned short m_serverPort;
    bool m_connected;

    OnPacketFn m_onPacketCallback;
    std::function<void()> m_onConnectedCallback;
    std::function<void()> m_onDisconnectedCallback;

    void handleIncomingPacket(sf::Packet& packet);

    uint32_t m_lastSequenceSent = 0;
    std::map<uint32_t, PendingPacket> m_pendingPackets;
};