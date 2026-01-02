#include "Entity System/EntityManager.h"
#include <algorithm> 

void EntityManager::addEntity(std::shared_ptr<Entity> entity, sf::Vector2f& spawnpoint) {
    entity->setPosition(spawnpoint);
    entities.push_back(entity);
}

void EntityManager::removeDeadEntities() {
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
    // Placeholder for future global updates
}