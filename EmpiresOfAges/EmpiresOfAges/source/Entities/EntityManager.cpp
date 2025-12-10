#include "Entity System/EntityManager.h"
#include <algorithm> // std::remove_if için

void EntityManager::addEntity(std::shared_ptr<Entity> entity, sf::Vector2f& spawnpoint) {
    entity->setPosition(spawnpoint);
    entities.push_back(entity);
}

void EntityManager::removeDeadEntities() {
    // Lambda fonksiyonu kullanarak canlý olmayanlarý vektörden siliyoruz
    entities.erase(
        std::remove_if(
            entities.begin(),
            entities.end(),
            [](const std::shared_ptr<Entity>& e) { return !e->getIsAlive(); }
        ),
        entities.end()
    );
}

void EntityManager::updateAll(float dt) {
    // Gelecekte entity'lerin kendi update fonksiyonu olursa burasý kullanýlýr
}