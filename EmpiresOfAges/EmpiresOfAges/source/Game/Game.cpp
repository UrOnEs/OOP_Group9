#include "Game/Game.h"

Game::Game() {
    // Pencereyi oluþturuyoruz (1280x720)
    window.create(sf::VideoMode(1280, 720), "RTS Oyunu");
    window.setFramerateLimit(60);
}

void Game::run() {
    sf::Clock clock;

    // OYUN DÖNGÜSÜ (Game Loop)
    while (window.isOpen()) {
        sf::Time dt = clock.restart(); // Delta time hesapla

        processEvents();       // 1. Girdileri al (Klavye/Fare)
        update(dt.asSeconds());// 2. Mantýðý güncelle (Hareket/Savaþ)
        render();              // 3. Ekrana çiz
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }
}

void Game::render() {
    window.clear(); // Ekraný temizle

    // player.renderEntities(window); // Oyuncularý çiz

    window.display(); // Ekrana yansýt
}

// update fonksiyonunu boþ býrakabilirsin þimdilik
void Game::update(float dt) {} 