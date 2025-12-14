#include "Entity System/Entity Type/Soldier.h"
#include "Game/GameRules.h"

int Soldier::IDcounter = 0;
int Soldier::counter = 0;

Soldier::Soldier() {
    this->entityID = ++IDcounter;
    counter++;

    // Baþlangýç pozisyonu (0,0 olmasýn)
    shape.setPosition(100.f, 100.f);

    // Varsayýlan Tip
    setType(SoldierTypes::Barbarian);
}

Soldier::~Soldier() {
    counter--;
}

void Soldier::setType(SoldierTypes type) {
    this->soldierType = type;

    switch (type) {
    case SoldierTypes::Barbarian:
        health = GameRules::HP_Barbarian;
        travelSpeed = GameRules::Speed_Barbarian;

        // DÜZELTME BURADA: setColor -> setFillColor
        shape.setFillColor(sf::Color::Red);
        break;

    case SoldierTypes::Archer:
        health = GameRules::HP_Archer;
        travelSpeed = GameRules::Speed_Archer;

        // DÜZELTME BURADA: setColor -> setFillColor
        shape.setFillColor(sf::Color::Green);
        break;

    case SoldierTypes::catapult:
        health = GameRules::HP_Catapult;
        travelSpeed = GameRules::Speed_Catapult;

        // DÜZELTME BURADA: setColor -> setFillColor
        shape.setFillColor(sf::Color::Magenta);
        break;
    }
}

std::string Soldier::stats() {
    return "HP: " + std::to_string((int)health);
}