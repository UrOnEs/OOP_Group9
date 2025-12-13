#include "Entity System/Entity Type/Soldier.h"
#include "Game/GameRules.h"

int Soldier::IDcounter = 0;
int Soldier::counter = 0;

Soldier::Soldier() {
    // Varsayýlan olarak Barbar özellikleriyle baþlasýn
    setType(SoldierTypes::Barbarian);

    this->isAlive = true;
    this->isSelected = false;
    this->entityID = ++IDcounter;
    counter++;
}

Soldier::~Soldier() {
    counter--;
}

// YENÝ FONKSÝYON: Tipe göre statlarý ayarla
void Soldier::setType(SoldierTypes type) {
    this->soldierType = type;

    switch (type) {
    case SoldierTypes::Barbarian:
        this->health = GameRules::HP_Barbarian;
        this->damage = GameRules::Dmg_Barbarian;
        this->travelSpeed = GameRules::Speed_Barbarian;
        this->range = GameRules::Range_Melee;
        break;

    case SoldierTypes::Archer:
        this->health = GameRules::HP_Archer;
        this->damage = GameRules::Dmg_Archer;
        this->travelSpeed = GameRules::Speed_Archer;
        this->range = GameRules::Range_Archer; // Uzun menzil
        break;

    case SoldierTypes::catapult:
        this->health = GameRules::HP_Catapult;
        this->damage = GameRules::Dmg_Catapult;
        this->travelSpeed = GameRules::Speed_Catapult; // Yavaþ
        this->range = GameRules::Range_Catapult; // Çok uzun menzil
        break;
    }
}

std::string Soldier::stats() {
    std::string name;
    if (soldierType == SoldierTypes::Barbarian) name = "Barbarian";
    else if (soldierType == SoldierTypes::Archer) name = "Archer";
    else name = "Catapult";

    return name + " HP:" + std::to_string((int)health) + " DMG:" + std::to_string((int)damage);
}