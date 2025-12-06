#include <SFML/Graphics.hpp>

#include "UIManager.h"
#include "UIPanel.h"
#include "UIButton.h"

// ---------------- GAME STATES ----------------
enum class MenuState {
    MAIN_MENU,
    LOBBY_MENU
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "RTS Demo - Main Menu");
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("assets/arial.ttf");

    MenuState currentState = MenuState::MAIN_MENU;

    // ---------------- MAIN MENU ----------------
    UIPanel mainMenu({ 400, 300 }, { 200, 150 });

    UIButton startButton({ 250, 50 }, { 275, 230 });
    startButton.setText("Start", font);
    startButton.setCallback([&]() {
        currentState = MenuState::LOBBY_MENU;
        });

    UIButton exitButton({ 250, 50 }, { 275, 300 });
    exitButton.setText("Exit", font);
    exitButton.setCallback([&window]() {
        window.close();
        });


    mainMenu.addButton(startButton);
    mainMenu.addButton(exitButton);

    // ---------------- LOBBY MENU ----------------
    UIPanel lobbyMenu({ 400, 300 }, { 200, 150 });

    UIButton joinLobby({ 250, 50 }, { 275, 230 });
    joinLobby.setText("Lobiye Baglan", font);
    joinLobby.setCallback([]() {
        // TODO: connect to lobby
        });

    UIButton createLobby({ 250, 50 }, { 275, 300 });
    createLobby.setText("Lobi Olustur", font);
    createLobby.setCallback([]() {
        // TODO: create lobby
        });

    UIButton backToMenu({ 250, 50 }, { 275, 370 });
    backToMenu.setText("Back", font);
    backToMenu.setCallback([&]() {
        currentState = MenuState::MAIN_MENU;
        });

    lobbyMenu.addButton(joinLobby);
    lobbyMenu.addButton(createLobby);
    lobbyMenu.addButton(backToMenu);

    // ---------------- MAIN LOOP ----------------
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            switch (currentState) {
            case MenuState::MAIN_MENU:
                mainMenu.handleEvent(event);
                break;

            case MenuState::LOBBY_MENU:
                lobbyMenu.handleEvent(event);
                break;
            }
        }

        window.clear(sf::Color(30, 30, 30));

        switch (currentState) {
        case MenuState::MAIN_MENU:
            mainMenu.draw(window);
            break;

        case MenuState::LOBBY_MENU:
            lobbyMenu.draw(window);
            break;
        }

        window.display();
    }

    return 0;
}

