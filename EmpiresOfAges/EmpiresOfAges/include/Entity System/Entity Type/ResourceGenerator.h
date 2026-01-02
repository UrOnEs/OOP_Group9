#pragma once
#include "Building.h"

/**
 * @brief Base class for entities that provide resources.
 * Handles worker garrison logic and periodic resource generation.
 */
class ResourceGenerator : public Building {
protected:
    int workerCount = 0;
    int maxWorkers = 1;

    float generateTimer = 0.0f;
    float interval = 3.0f;
    int amountPerTick = 10;
    int remainingResources = -1; // -1 means infinite

public:
    bool garrisonWorker() {
        if (workerCount < maxWorkers) {
            workerCount++;
            generateTimer = interval;
            return true;
        }
        return false;
    }

    void releaseWorker() {
        if (workerCount > 0) {
            workerCount--;
        }
    }

    /**
     * @brief Updates resource generation logic.
     * @return Amount of resource generated in this tick.
     */
    int updateGeneration(float dt) {
        if (workerCount > 0) {
            generateTimer -= dt;
            if (generateTimer <= 0) {
                generateTimer = interval;

                int totalProduction = amountPerTick * workerCount;

                // Finite resources check
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
                // Infinite resources
                else if (remainingResources == -1) {
                    return totalProduction;
                }
            }
        }
        return 0;
    }

    void setTotalResources(int amount) { remainingResources = amount; }
    bool isWorking() const { return workerCount > 0; }
    bool isFull() const { return workerCount >= maxWorkers; }
};