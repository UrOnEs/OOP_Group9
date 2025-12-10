#include "Entity System/Entity Type/Soldier.h"
#include "Game/GameRules.h"
#include <string>

// Static deðiþkenlerin tanýmlanmasý (Header'da sadece declare edilmiþtir)
int Soldier::IDcounter = 0;
int Soldier::counter = 0;

Soldier::Soldier() {
    // GameRules namespace'inden verileri çekiyoruz
    this->health = GameRules::SoldierHealth;
    this->damage = GameRules::SoldierDamage;
    this->travelSpeed = GameRules::SoldierSpeed;
    this->range = GameRules::MeleeRange; // Okçuysa deðiþebilir, þimdilik default

    this->isAlive = true;
    this->isSelected = false;

    // Her askere benzersiz bir ID ver
    this->entityID = ++IDcounter;
    counter++;

    // Baþlangýçta boþ bir texture atanabilir veya Player sýnýfý bunu setTexture ile düzeltecek
}

Soldier::~Soldier() {
    counter--;
}

// Entity.h'daki abstract fonksiyonu override ediyoruz
std::string Soldier::stats() {
    return "Soldier HP: " + std::to_string(health) + " DMG: " + std::to_string(damage);
}