#pragma once
#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include <SFML/System/Vector2.hpp>

class EntityManager {
private:

    std::vector<std::shared_ptr<Entity>> entities;

public:
    
    void addEntity(std::shared_ptr<Entity> entity, sf::Vector2f& spawnpoint);
    
    void updateAll(float dt);
    
    void removeDeadEntities();

};

