#pragma once
#include "Entity System/Entity.h"
#include "types.h"

class Building : public Entity {
public:
    BuildTypes buildingType;

    Building() {
        health = 500.f;
    }

    virtual ~Building() = default;

    // --- BURAYI DEÐÝÞTÝRÝYORUZ ---

    // Entity'den gelen borcu burada ödüyoruz.
    // Artýk Farm veya House tekrar stats() yazmak zorunda kalmayacak.
    std::string stats() override {
        return getInfo(); // stats istendiðinde getInfo'yu çaðýrýp döndürsün.
    }

    // Alt sýnýflar bunu doldurmak ZORUNDA (Bunu zaten yapýyorlar, sorun yok)
    virtual std::string getInfo() = 0;
};