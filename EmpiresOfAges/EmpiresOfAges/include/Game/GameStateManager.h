#pragma once

#include "GameState.h"

class GameStateManager {
public:
    void setState(GameState state);
    GameState getState() const;

    void update();

private:
    GameState currentState;
};

