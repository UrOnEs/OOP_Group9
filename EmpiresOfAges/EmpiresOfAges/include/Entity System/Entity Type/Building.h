#pragma once
#include "Entity System/Entity.h"
#include "types.h"
#include "Game/GameRules.h" // GameRules eklendi

class Building : public Entity {
public:
    BuildTypes buildingType;

    Building() {
        // health = 500.f;  <-- BUNU SÝLÝYORUZ. 
        // Artýk varsayýlan can yok, alt sýnýflar atayacak.
        // Ama güvenlik için yine de bir baþlangýç deðeri verelim:
        health = GameRules::BuildingHealth;
    }

    virtual ~Building() = default;

    std::string stats() override {
        return getInfo();
    }

    virtual std::string getInfo() = 0;
};