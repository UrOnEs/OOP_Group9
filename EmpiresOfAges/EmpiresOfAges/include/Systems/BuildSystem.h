#pragma once

#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/Building.h"
#include <SFML/System/Vector2.hpp>

class Player; //include etmiþ gibi biþey oluyoruz

class BuildSystem {
public:
    static bool build(Player& player, Villager& worker, Building& building, const sf::Vector2f& pos); // Basitçe ( Hangi oyuncu, Hangi iþçi , hangi bina , nereye)

private:
    static bool checkSpace(Building& building, const sf::Vector2f& pos); //yer boþ mu
};