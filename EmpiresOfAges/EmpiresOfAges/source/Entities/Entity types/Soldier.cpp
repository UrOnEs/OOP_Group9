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

        shape.setFillColor(sf::Color::Red);
        break;

    case SoldierTypes::Archer:
        health = GameRules::HP_Archer;
        travelSpeed = GameRules::Speed_Archer;


        shape.setFillColor(sf::Color::Green);
        break;

    case SoldierTypes::Wizard: // Catapult gitti, Wizard geldi
        health = GameRules::HP_Wizard;
        travelSpeed = GameRules::Speed_Wizard;
        shape.setFillColor(sf::Color::Blue); // Büyücü Mavi olsun
        break;
    }
}

std::string Soldier::stats() {
    return "HP: " + std::to_string((int)health);
}