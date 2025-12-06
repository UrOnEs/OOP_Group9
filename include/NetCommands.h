#pragma once
#pragma once
#pragma once
#include <cstdint>

enum class NetCommand : uint16_t {
    None = 0,
    RequestJoin = 1,
    AcceptJoin = 2,
    SyncState = 10,
    IssueOrder = 20,
    ChatMessage = 30,
    // ... oyun ihtiya�lar�na g�re
};
