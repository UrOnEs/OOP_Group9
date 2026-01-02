#pragma once
#include <cstdint>
#include <map>
#include <SFML/System/Clock.hpp>
#include <SFML/Network/Packet.hpp>

/**
 * @brief Enum representing various network commands used in the game.
 */
enum class NetCommand : uint16_t {
    None = 0,
    RequestJoin = 1,
    AcceptJoin = 2,
    SyncState = 10,
    IssueOrder = 20,
    ChatMessage = 30,
    TrainUnit = 40,
    PlaceBuilding = 50
    // Add more game-specific commands here
};

/**
 * @brief Categorizes packets based on their delivery requirements.
 */
enum class PacketType : uint8_t {
    Unreliable = 0, ///< Fire-and-forget packets (e.g., position updates).
    Reliable = 1,   ///< Critical packets that must arrive (e.g., orders).
    ACK = 2         ///< Acknowledgment packets.
};

/**
 * @brief Structure to track packets waiting for acknowledgment.
 */
struct PendingPacket {
    sf::Packet packet;
    sf::Clock timer;
    uint32_t sequence;
};