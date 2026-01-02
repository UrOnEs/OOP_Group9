#include "Game/GameStateManager.h"

void GameStateManager::setState(GameState state) {
    currentState = state;
}

GameState GameStateManager::getState() const {
    return currentState;
}

void GameStateManager::update() {
    // State transitions or animations could be handled here
}