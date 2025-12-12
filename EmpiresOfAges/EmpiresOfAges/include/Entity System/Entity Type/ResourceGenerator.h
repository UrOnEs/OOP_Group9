#pragma once
#include "Building.h"

class ResourceGenerator : public Building {
protected:
    bool hasWorker = false;
    float generateTimer = 0.0f;
    float interval = 3.0f; // 3 saniyede bir kaynak versin
    int amountPerTick = 10; // Her seferinde ne kadar versin?

public:
    // Ýþçi içeri giriyor
    bool garrisonWorker() {
        if (!hasWorker) {
            hasWorker = true;
            generateTimer = interval; // Sayacý baþlat
            return true; // Baþarýlý giriþ
        }
        return false; // Zaten dolu
    }

    // Ýþçi dýþarý çýkýyor (Örn: bina yýkýlýrsa veya oyuncu isterse)
    void releaseWorker() {
        hasWorker = false;
        // Burada yeni bir Villager oluþturup kapýya koyman gerekir
    }

    // Sistem tarafýndan çaðrýlacak güncelleme
    // Dönüþ deðeri: Ne kadar kaynak ürettiði (0 ise henüz üretmedi)
    int updateGeneration(float dt) {
        if (hasWorker) {
            generateTimer -= dt;
            if (generateTimer <= 0) {
                generateTimer = interval; // Sayacý sýfýrla
                return amountPerTick; // Kaynaðý ver
            }
        }
        return 0;
    }

    bool isWorking() const { return hasWorker; }
};
