#include "Entity System/Entity Type/Villager.h"
#include "Game/GameRules.h"

int Villager::IDcounter = 0;
int Villager::counter = 0;

Villager::Villager() {
    this->health = GameRules::VillagerHealth;
    this->damage = GameRules::VillagerDamage;
    this->travelSpeed = GameRules::VillagerSpeed;
    this->range = GameRules::MeleeRange;

    this->isAlive = true;
    this->isSelected = false;
    this->entityID = ++IDcounter;

    counter++;
}

Villager::~Villager() {
    counter--;
}

std::string Villager::stats() {
    return "Villager HP: " + std::to_string(health);
}