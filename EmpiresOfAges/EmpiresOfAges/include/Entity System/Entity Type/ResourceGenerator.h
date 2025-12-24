#pragma once
#include "Building.h"

class ResourceGenerator : public Building {
protected:
    int workerCount = 0;      // Ýçerideki iþçi sayýsý
    int maxWorkers = 1;       // Kapasite (Aðaç için 1, Çiftlik için 4 olacak)

    float generateTimer = 0.0f;
    float interval = 3.0f;
    int amountPerTick = 10;
    int remainingResources = -1;

public:
    // --- GÜNCELLENEN GÝRÝÞ MANTIÐI ---
    bool garrisonWorker() {
        if (workerCount < maxWorkers) {
            workerCount++;
            generateTimer = interval; // Sayacý tetikle
            return true; // Giriþ baþarýlý
        }
        return false; // Dolu
    }

    // --- GÜNCELLENEN ÇIKIÞ MANTIÐI ---
    void releaseWorker() {
        if (workerCount > 0) {
            workerCount--;
        }
        // Eðer bina yýkýldýysa veya boþaltýldýysa dýþarý köylü spawn etme mantýðý 
        // Villager sýnýfýnda veya binanýn yýkýlma anýnda (Game.cpp) yönetilmeli.
    }

    // --- GÜNCELLENEN ÜRETÝM MANTIÐI ---
    int updateGeneration(float dt) {
        if (workerCount > 0) {
            generateTimer -= dt;
            if (generateTimer <= 0) {
                generateTimer = interval;

                // FORMÜL: Temel Üretim * Ýþçi Sayýsý
                int totalProduction = amountPerTick * workerCount;

                // Sýnýrlý Kaynak Kontrolü (Aðaç vb.)
                if (remainingResources > 0) {
                    if (remainingResources < totalProduction) {
                        totalProduction = remainingResources;
                    }
                    remainingResources -= totalProduction;

                    if (remainingResources <= 0) {
                        this->health = 0;
                        this->isAlive = false;
                        workerCount = 0;
                    }
                    return totalProduction;
                }
                // Sýnýrsýz Kaynak (Çiftlik)
                else if (remainingResources == -1) {
                    return totalProduction;
                }
            }
        }
        return 0;
    }

    void setTotalResources(int amount) { remainingResources = amount; }

    // Artýk "En az 1 kiþi varsa" çalýþýyor demektir
    bool isWorking() const { return workerCount > 0; }

    // Dolu mu?
    bool isFull() const { return workerCount >= maxWorkers; }
};