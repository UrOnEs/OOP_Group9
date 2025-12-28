#ifndef SOLDIER_H
#define SOLDIER_H

#include "Entity System/Entity Type/types.h"
#include "Entity System/Entity Type/Unit.h"
#include "Game/GameRules.h"
#include <memory>
#include <vector>

enum class SoldierState { Idle, Moving, Chasing, Attacking };

class Soldier : public Unit {
private:
    static int IDcounter;
    static int counter;
    SoldierTypes soldierType;

    SoldierState state = SoldierState::Idle;
    std::weak_ptr<Entity> targetEntity;

    float attackTimer = 0.0f;
    float attackInterval = 1.0f;
    float attackRange = 20.0f;

    // Büyücü Deðiþkenleri
    float wizardChargeTimer = 0.0f;
    float wizardMaxChargeTime = GameRules::AttackSpeed_Wizard;
    bool isCharging = false;

public:
    Soldier();
    ~Soldier();

    void setType(SoldierTypes type);
    int getMaxHealth() const override;
    std::string stats() override;

    void setTarget(std::shared_ptr<Entity> target);
    void clearTarget();
    bool hasTarget() const { return state != SoldierState::Idle; }

    void updateSoldier(float dt, const std::vector<std::shared_ptr<Entity>>& potentialTargets);

    void render(sf::RenderWindow& window) override;
    void renderEffects(sf::RenderWindow& window) override;

    std::string getName() override;
    sf::Texture* getIcon() override;
};

#endif