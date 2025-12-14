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
    std::string stats() override;

    // --- ESKÝ KODLARIN ÇALIÞMASI ÝÇÝN YÖNLENDÝRME ---
    // Game.cpp "getModel()" diye sorarsa, babasýndaki "getShape()"i versin.
    sf::CircleShape& getModel() { return getShape(); }
};

#endif