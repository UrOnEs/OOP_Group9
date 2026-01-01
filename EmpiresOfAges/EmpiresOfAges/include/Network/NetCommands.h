#pragma once
#pragma once
#pragma once
#include <cstdint>
#include <map>
#include <SFML/System/Clock.hpp>
#include <SFML/Network/Packet.hpp>

enum class NetCommand : uint16_t {
    None = 0,
    RequestJoin = 1,
    AcceptJoin = 2,
    SyncState = 10,
    IssueOrder = 20,
    ChatMessage = 30,
    TrainUnit = 40,
    PlaceBuilding = 50
    // ... oyun ihtiya�lar�na g�re
};
enum class PacketType : uint8_t {
    Unreliable = 0, // Pozisyon güncellemeleri gibi kaybolsa da sorun olmayanlar
    Reliable = 1,   // Emirler gibi mutlaka gitmesi gerekenler
    ACK = 2         // "Aldım" onayı
};

struct PendingPacket {
    sf::Packet packet;
    sf::Clock timer;
    uint32_t sequence;
};
