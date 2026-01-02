#pragma once

/**
 * @brief Represents the high-level state of the game application.
 */
enum class GameState {
    Menu,           ///< Main Menu
    LobbyRoom,      ///< Waiting in lobby
    LobbySelection, ///< Searching for lobbies
    Playing,        ///< Active gameplay
    GameOver        ///< Match finished
};