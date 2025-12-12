#include "Game/GameStateManager.h"

void GameStateManager::setState(GameState state) {
    currentState = state;
}

GameState GameStateManager::getState() const {
    return currentState;
}

void GameStateManager::update() {
    // Şimdilik boş kalabilir, ileride state geçiş efektleri buraya gelir.
}