#ifndef SOLDIER_H
#define SOLDIER_H

#include "Entity System/Entity Type/types.h"
#include "Unit.h"

class Soldier : public Unit {
private:
    static int IDcounter;
    static int counter;
public:
    SoldierTypes soldierType;

    Soldier(); // Constructor (GameRules verilerini buradan yüklicez)

    ~Soldier();

};

#endif // !SOLDIER_H