#pragma once
#include "GameState.h"

/**
 * @brief Manages the current game state and transitions.
 */
class GameStateManager {
public:
    void setState(GameState state);
    GameState getState() const;
    void update();

private:
    GameState currentState;
};