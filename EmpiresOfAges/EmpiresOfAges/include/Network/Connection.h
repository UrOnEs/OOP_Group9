#pragma once
#include <SFML/Network.hpp>
#include <string>
#include <map>
#include "NetCommands.h"

/**
 * @brief Represents a connection state.
 * In UDP, 'Connected' implies we have a target IP/Port.
 */
enum class ConnectionState { Disconnected, Connected };

/**
 * @brief Manages a logical connection over UDP, handling packet sequencing and reliability.
 */
class Connection {
public:
    /**
     * @brief Constructs a connection to a specific IP and port.
     * @param address Target IP address.
     * @param port Target port number.
     */
    Connection(const sf::IpAddress& address, unsigned short port);

    /**
     * @brief Sends a packet through the provided UDP socket.
     * @param socket The physical UDP socket to use for sending.
     * @param packet The data packet to send.
     * @return sf::Socket::Status Status of the send operation.
     */
    sf::Socket::Status send(sf::UdpSocket& socket, sf::Packet& packet) const;

    /**
     * @brief Returns the connection state.
     * UDP assumes connectivity if target address is valid.
     */
    ConnectionState state() const { return ConnectionState::Connected; }

    /**
     * @brief Returns a string representation of the endpoint (IP:Port).
     */
    std::string endpoint() const;

    // --- Accessors ---
    sf::IpAddress getAddress() const { return m_address; }
    unsigned short getPort() const { return m_port; }

    /**
     * @brief Retrieves the next sequence number for reliable transmission.
     */
    uint32_t getNextSequence() { return ++m_lastSequenceSent; }

    /**
     * @brief Adds a packet to the pending list for reliable delivery assurance.
     */
    void addPendingPacket(uint32_t seq, sf::Packet pkt);

    /**
     * @brief Processes an acknowledgment, removing the packet from the pending list.
     */
    void processACK(uint32_t seq);

    /**
     * @brief Resends packets that haven't received an ACK within the timeout period.
     */
    void resendMissingPackets(sf::UdpSocket& socket);

private:
    sf::IpAddress m_address;
    unsigned short m_port;

    // Optional statistics
    uint64_t m_bytesSent = 0;
    uint32_t m_lastSequenceSent = 0;
    std::map<uint32_t, PendingPacket> m_pendingPackets; ///< Packets waiting for ACK.
};