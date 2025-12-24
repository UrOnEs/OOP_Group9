#include "Entity System/Entity Type/Soldier.h"
#include "Game/GameRules.h"

int Soldier::IDcounter = 0;
int Soldier::counter = 0;

Soldier::Soldier() {
    this->entityID = ++IDcounter;
    counter++;

    // Baþlangýç pozisyonu (0,0 olmasýn)
    shape.setPosition(100.f, 100.f);

    this->setTexture(AssetManager::getTexture("assets/units/barbarian.png"));
    float scaleX = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().x;
    float scaleY = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().y;
    this->setScale(scaleX, scaleY);

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

        
        //shape.setFillColor(sf::Color::Red);
        break;

    case SoldierTypes::Archer:
        health = GameRules::HP_Archer;
        travelSpeed = GameRules::Speed_Archer;

        this->setTexture(AssetManager::getTexture("assets/units/archer.png"));
        //shape.setFillColor(sf::Color::Green);
        break;

    case SoldierTypes::Wizard:
        health = GameRules::HP_Wizard;
        travelSpeed = GameRules::Speed_Wizard;

        this->setTexture(AssetManager::getTexture("assets/units/wizard.png"));
        //shape.setFillColor(sf::Color::Blue); 
        break;
    }
}

int Soldier::getMaxHealth() const {
    switch (soldierType) {
    case SoldierTypes::Barbarian: return (int)GameRules::HP_Barbarian;
    case SoldierTypes::Archer:    return (int)GameRules::HP_Archer;
    case SoldierTypes::Wizard:    return (int)GameRules::HP_Wizard;
    default:                      return 100;
    }
}

std::string Soldier::stats() {
    return "HP: " + std::to_string((int)health);
}