#pragma once
#include "Building.h"

class ResourceGenerator : public Building {
protected:
    bool hasWorker = false;
    float generateTimer = 0.0f;
    float interval = 3.0f; // 3 saniyede bir kaynak versin
    int amountPerTick = 10; // Her seferinde ne kadar versin?

    int remainingResources = -1;

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
                generateTimer = interval;

                // Eðer kaynak sýnýrlýysa (Örn: Aðaç)
                if (remainingResources > 0) {
                    int amountToGive = amountPerTick;

                    // Kalan miktar istenenden azsa, kalaný ver
                    if (remainingResources < amountToGive) {
                        amountToGive = remainingResources;
                    }

                    remainingResources -= amountToGive;

                    // Kaynak bittiyse binayý/aðacý yok et!
                    if (remainingResources <= 0) {
                        this->health = 0; // Caný sýfýrla, EntityManager bunu silecek
                        this->isAlive = false;
                        hasWorker = false; // Ýþçiyi boþa çýkar
                    }

                    return amountToGive;
                }
                // Sýnýrsýz kaynaklar için (Örn: Tarla sonsuz olabilir)
                else if (remainingResources == -1) {
                    return amountPerTick;
                }
            }
        }
        return 0;
    }

    // Kaynak miktarýný ayarlamak için setter
    void setTotalResources(int amount) { remainingResources = amount; }


bool isWorking() const { return hasWorker; }
};
