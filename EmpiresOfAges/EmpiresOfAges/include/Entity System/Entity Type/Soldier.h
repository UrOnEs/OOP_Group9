#ifndef SOLDIER_H
#define SOLDIER_H

#include "Entity System/Entity Type/types.h"
#include "Unit.h"

class Soldier : public Unit {
private:
    static int IDcounter;
    static int counter;
public:

    void setType(SoldierTypes type);

    std::string stats() override;

    SoldierTypes soldierType;

    Soldier(); // Constructor (GameRules verilerini buradan yüklicez)

    ~Soldier();

    void render(sf::RenderWindow& window) override;
};

#endif // !SOLDIER_H