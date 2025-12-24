#ifndef SOLDIER_H
#define SOLDIER_H

#include "Entity System/Entity Type/types.h"
#include "Entity System/Entity Type/Unit.h"

class Soldier : public Unit {
private:
    static int IDcounter;
    static int counter;
    SoldierTypes soldierType;

public:
    Soldier();
    ~Soldier();

    void setType(SoldierTypes type);
    int getMaxHealth() const override;
    std::string stats() override;

    // --- ESKÝ KODLARIN ÇALIÞMASI ÝÇÝN YÖNLENDÝRME ---
    // Game.cpp "getModel()" diye sorarsa, babasýndaki "getShape()"i versin.
    sf::CircleShape& getModel() { return getShape(); }

    std::string getName() override {
        switch (soldierType) {
        case SoldierTypes::Barbarian: return "Barbarian";
        case SoldierTypes::Archer:    return "Archer";
        case SoldierTypes::Wizard:    return "Wizard";
        default:                      return "Soldier";
        }
    }

    sf::Texture* getIcon() override {
        if (hasTexture) return (sf::Texture*)sprite.getTexture();
        return nullptr;
    }
};

#endif