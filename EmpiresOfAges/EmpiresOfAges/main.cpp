
// main.cpp
#include "Game/Game.h"
#include <iostream>

int main() {
    try {
        std::cout << "Empires of Ages baslatiliyor..." << std::endl;

        Game game;
        game.run();

    }
    catch (const std::exception& e) {
        std::cerr << "KRITIK HATA: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}