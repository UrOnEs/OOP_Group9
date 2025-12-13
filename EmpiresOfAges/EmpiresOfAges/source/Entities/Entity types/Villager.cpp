#include "Entity System/Entity Type/Villager.h"
#include "Game/GameRules.h"

// Static üyeleri baþlat
int Villager::IDcounter = 0;
int Villager::counter = 0;

Villager::Villager() {
    // GameRules'dan verileri çek
    this->health = GameRules::HP_Villager;
    this->damage = GameRules::Dmg_Villager;
    this->travelSpeed = GameRules::Speed_Villager;
    this->range = GameRules::Range_Melee;

    this->isAlive = true;
    this->isSelected = false;
    this->entityID = ++IDcounter;

    counter++;
}

Villager::~Villager() {
    counter--;
}

std::string Villager::stats() {
    return "Villager HP: " + std::to_string((int)health);
}