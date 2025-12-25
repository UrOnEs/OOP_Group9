#include "Entity System/Entity Type/Soldier.h"
#include "Game/GameRules.h"
#include "Systems/CombatSystem.h" // Saldýrý sistemi için gerekli
#include <iostream>
#include <cmath>

int Soldier::IDcounter = 0;
int Soldier::counter = 0;

Soldier::Soldier() {
    this->entityID = ++IDcounter;
    counter++;

    // Baþlangýç pozisyonu
    shape.setPosition(100.f, 100.f);

    // --- TAKIM ARKADAÞININ EKLEDÝÐÝ GÖRSEL KODLAR (KORUNDU) ---
    this->setTexture(AssetManager::getTexture("assets/units/barbarian.png"));
    // Görsel boyutlandýrma
    if (this->sprite.getTexture()) {
        float scaleX = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().x;
        float scaleY = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().y;
        this->setScale(scaleX, scaleY);
    }
    // -----------------------------------------------------------

    // Varsayýlan Tip
    setType(SoldierTypes::Barbarian);
}

Soldier::~Soldier() {
    counter--;
}

void Soldier::setType(SoldierTypes type) {
    this->soldierType = type;

    // Deðerleri GameRules'dan çekiyoruz
    switch (type) {
    case SoldierTypes::Barbarian:
        health = GameRules::HP_Barbarian;
        travelSpeed = GameRules::Speed_Barbarian;
        attackInterval = GameRules::AttackSpeed_Barbarian;
        attackRange = GameRules::Range_Melee;
        this->damage = GameRules::Dmg_Barbarian;
        // Görsel güncelleme (Eðer her tip için farklýysa burayý açabilirsin)
        // this->setTexture(AssetManager::getTexture("assets/units/barbarian.png"));
        this->range = attackRange;
        break;

    case SoldierTypes::Archer:
        health = GameRules::HP_Archer;
        travelSpeed = GameRules::Speed_Archer;
        attackInterval = GameRules::AttackSpeed_Archer;
        attackRange = GameRules::Range_Archer;
        this->setTexture(AssetManager::getTexture("assets/units/archer.png"));
        this->range = attackRange;
        this->damage = GameRules::Dmg_Archer;
        break;

    case SoldierTypes::Wizard:
        health = GameRules::HP_Wizard;
        travelSpeed = GameRules::Speed_Wizard;
        attackInterval = GameRules::AttackSpeed_Wizard;
        attackRange = GameRules::Range_Wizard;
        this->setTexture(AssetManager::getTexture("assets/units/wizard.png"));
        this->range = attackRange;
        this->damage = GameRules::Dmg_Wizard;
        break;
    }
}

int Soldier::getMaxHealth() const {
    switch (soldierType) {
    case SoldierTypes::Barbarian: return (int)GameRules::HP_Barbarian;
    case SoldierTypes::Archer:    return (int)GameRules::HP_Archer;
    case SoldierTypes::Wizard:    return (int)GameRules::HP_Wizard;
    default:                      return 100;
    }
}

std::string Soldier::stats() {
    return "HP: " + std::to_string((int)health);
}

// =========================================================
//                  SAVAÞ MANTIÐI (COMBAT AI)
// =========================================================

void Soldier::setTarget(std::shared_ptr<Entity> target) {
    if (target) {
        // --- DOST ATEÞÝ KONTROLÜ -----------
        // Eðer hedef benim takýmdansa SALDIRMA.
        if (target->getTeam() == this->getTeam()) {
            // Ýstersen buraya bir log koyabilirsin:
            // std::cout << "[SOLDIER] Dost birim, saldirilamaz!\n";
            return;
        }

        // Eðer zaten ayný hedefe saldýrýyorsam tekrar emir verme (Log kirliliðini önler)
        auto currentTarget = targetEntity.lock();
        if (currentTarget && currentTarget == target) {
            return;
        }

        targetEntity = target;
        state = SoldierState::Chasing;
        std::cout << "[SOLDIER #" << entityID << "] Hedef belirlendi! Kovaliyorum...\n";
    }
}

void Soldier::clearTarget() {
    targetEntity.reset();
    state = SoldierState::Idle;
    m_path.clear();
    m_isMoving = false;
}

void Soldier::updateSoldier(float dt, const std::vector<std::shared_ptr<Entity>>& potentialTargets) {    // Saldýrý sayacýný her zaman çalýþtýr (Cooldown)
    // ---  ZOMBÝ KONTROLÜ ---
    // Eðer asker öldüyse hiçbir iþlem yapma!
    if (!getIsAlive()) {
        state = SoldierState::Idle;
        return;
    }
    // -----------------------------------
    
    if (attackTimer > 0) attackTimer -= dt;

    // =========================================================
    //           0. Durum: IDLE (Boþta) -> OTOMATÝK HEDEF ARAMA
    // =========================================================
    if (state == SoldierState::Idle) {
        // Görüþ Menzili (GameRules::Range_Sight_Normal = 300.f varsayalým)
        float scanRange = 50.0f;
        float bestDist = scanRange;
        std::shared_ptr<Entity> bestTarget = nullptr;

        for (const auto& target : potentialTargets) {
            // Ölüleri ve kendi takýmýmýzý es geç
            if (!target || !target->getIsAlive()) continue;
            if (target->getTeam() == this->getTeam()) continue;

            // Mesafe kontrolü
            sf::Vector2f diff = target->getPosition() - getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;

            // Karekök iþlemi pahalýdýr, o yüzden kareleriyle kýyaslamak daha performanslýdýr
            if (distSq < bestDist * bestDist) {
                bestDist = std::sqrt(distSq);
                bestTarget = target;
            }
        }

        // Eðer uygun bir düþman bulduysak SALDIR!
        if (bestTarget) {
            setTarget(bestTarget);
        }
    }

    // 1. Durum: HEDEF KOVALAMA (Chasing)
    if (state == SoldierState::Chasing) {
        auto target = targetEntity.lock();

        // Hedef yok olduysa veya öldüyse
        if (!target || !target->getIsAlive()) {
            std::cout << "[SOLDIER #" << entityID << "] Hedefim yok oldu. Beklemeye geciyorum.\n";
            clearTarget();
            return;
        }

        // Mesafe Kontrolü
        sf::Vector2f diff = target->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        // Hedefin görsel geniþliðinin yarýsýný al (Kabaca yarýçap)
        float targetRadius = target->getBounds().width / 2.0f;

        // Çok küçük hedefler için minimum bir deðer belirle (Hata payý)
        if (targetRadius < 15.0f) targetRadius = 15.0f;

        // Menzil içinde miyiz? (Saldýrý Menzili + Biraz tolerans)
        float attackReach = attackRange + targetRadius + 10.0f;
        if (dist <= attackReach) {
            // Menzildeyiz -> Saldýrý Moduna Geç
            state = SoldierState::Attacking;

            // Hareketi durdur (Vururken yürümesin)
            m_path.clear();
            m_isMoving = false;

            std::cout << "[SOLDIER #" << entityID << "] Menzilde! Saldiriya geciliyor.\n";
        }
        else {
            // Menzilde deðiliz -> Yürümeye devam et
            moveTo(target->getPosition());
        }
    }

    // 2. Durum: SALDIRI (Attacking)
    else if (state == SoldierState::Attacking) {
        auto target = targetEntity.lock();
        if (!target || !target->getIsAlive()) {
            clearTarget();
            return;
        }

        // Menzilden çýkma kontrolü (Hysteresis)
        sf::Vector2f diff = target->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        float targetRadius = target->getBounds().width / 2.0f;
        if (targetRadius < 15.0f) targetRadius = 15.0f;

        // Saldýrýdan kopma mesafesi
        float stopAttackDist = attackRange + targetRadius + 20.0f;

        if (dist > stopAttackDist) {
            state = SoldierState::Chasing; // Tekrar kovalamaya dön
            return;
        }

        if (attackTimer <= 0) {
            // --- 2. DÜZELTME: VURUÞ BAÞARISIZSA YÜRÜ ---
            // CombatSystem menzil kontrolü yapar. Eðer menzil yetmezse false döner.
            bool hitSuccess = CombatSystem::attack(*this, *target);

            if (hitSuccess) {
                attackTimer = attackInterval;
            }
            else {
                // Menzil yetmediyse (CombatSystem reddettiyse) durma, KOVALA!
                state = SoldierState::Chasing;
            }
        }
    }
}