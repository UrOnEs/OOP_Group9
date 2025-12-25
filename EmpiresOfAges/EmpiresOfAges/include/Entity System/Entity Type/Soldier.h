#ifndef SOLDIER_H
#define SOLDIER_H

#include "Entity System/Entity Type/types.h"
#include "Entity System/Entity Type/Unit.h"
#include <memory> // weak_ptr için
#include <vector>

// Askerin Ruh Halleri
enum class SoldierState {
    Idle,       // Boþta, emir bekliyor
    Moving,     // Bir konuma yürüyor (Saldýrmadan)
    Chasing,    // Bir hedefi kovalýyor
    Attacking   // Vuruyor
};

class Soldier : public Unit {
private:
    static int IDcounter;
    static int counter;
    SoldierTypes soldierType;

    // --- SAVAÞ DEÐÝÞKENLERÝ ---
    SoldierState state = SoldierState::Idle;
    std::weak_ptr<Entity> targetEntity; // Hedef (Düþman, Bina vs.)

    float attackTimer = 0.0f;     // Vurmak için geri sayým
    float attackInterval = 1.0f;  // Saldýrý hýzý (Saniyede kaç vuruþ?)
    float attackRange = 20.0f;    // Vuruþ menzili

public:
    Soldier();
    ~Soldier();

    void setType(SoldierTypes type);
    int getMaxHealth() const override;
    std::string stats() override;
    
    void setTarget(std::shared_ptr<Entity> target);
    void clearTarget();

    // Askerin þu an bir hedefi var mý?
    bool hasTarget() const { return state != SoldierState::Idle; }

    // Askerin beyni (Her karede ne yapacaðýna karar verir)
    void updateSoldier(float dt, const std::vector<std::shared_ptr<Entity>>& potentialTargets);
    sf::CircleShape& getModel() { return getShape(); }
};

#endif