#include "Entity System/Entity Type/Soldier.h"
#include "Game/GameRules.h"
#include "Systems/CombatSystem.h" 
#include <iostream>
#include <cmath>

int Soldier::IDcounter = 0;
int Soldier::counter = 0;

Soldier::Soldier() {
    this->entityID = ++IDcounter;
    counter++;
    shape.setPosition(100.f, 100.f);
    this->setTexture(AssetManager::getTexture("assets/units/barbarian.png"));

    // Görsel boyutlandýrma
    if (this->sprite.getTexture()) {
        float scaleX = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().x;
        float scaleY = (GameRules::UnitRadius * 5) / (float)this->sprite.getTexture()->getSize().y;
        this->setScale(scaleX, scaleY);
    }
    setType(SoldierTypes::Barbarian);
}

Soldier::~Soldier() { counter--; }

void Soldier::setType(SoldierTypes type) {
    this->soldierType = type;

    switch (type) {
    case SoldierTypes::Barbarian:
        health = GameRules::HP_Barbarian;
        travelSpeed = GameRules::Speed_Barbarian;
        attackInterval = GameRules::AttackSpeed_Barbarian;
        attackRange = GameRules::Range_Melee;
        this->damage = GameRules::Dmg_Barbarian;
        this->range = attackRange;
        break;

    case SoldierTypes::Archer:
        health = GameRules::HP_Archer;
        travelSpeed = GameRules::Speed_Archer;
        attackInterval = GameRules::AttackSpeed_Archer;
        attackRange = GameRules::Range_Archer;
        this->damage = GameRules::Dmg_Archer;
        this->range = attackRange;
        this->setTexture(AssetManager::getTexture("assets/units/archer.png"));
        break;

    case SoldierTypes::Wizard:
        health = GameRules::HP_Wizard;
        travelSpeed = GameRules::Speed_Wizard;
        attackInterval = GameRules::AttackSpeed_Wizard;
        attackRange = GameRules::Range_Wizard;
        this->damage = GameRules::Dmg_Wizard;
        this->range = attackRange;

        // Þarj süresini GameRules'dan alýyoruz
        this->wizardMaxChargeTime = attackInterval;

        this->setTexture(AssetManager::getTexture("assets/units/wizard.png"));
        break;
    }
}

int Soldier::getMaxHealth() const {
    switch (soldierType) {
    case SoldierTypes::Barbarian: return (int)GameRules::HP_Barbarian;
    case SoldierTypes::Archer:    return (int)GameRules::HP_Archer;
    case SoldierTypes::Wizard:    return (int)GameRules::HP_Wizard;
    default: return 100;
    }
}

std::string Soldier::stats() { return "HP: " + std::to_string((int)health); }

std::string Soldier::getName() {
    switch (soldierType) {
    case SoldierTypes::Barbarian: return "Barbarian";
    case SoldierTypes::Archer:    return "Archer";
    case SoldierTypes::Wizard:    return "Wizard";
    default: return "Soldier";
    }
}

// --- HEDEFLEME ---
void Soldier::setTarget(std::shared_ptr<Entity> target) {
    if (target) {
        if (target->getTeam() == this->getTeam()) return; // Dost ateþi yok

        // Zaten ayný hedefe saldýrýyorsak tekrar atama yapma
        auto current = targetEntity.lock();
        if (current && current == target) return;

        targetEntity = target;
        state = SoldierState::Chasing;
    }
}

void Soldier::clearTarget() {
    targetEntity.reset();
    state = SoldierState::Idle;
    isCharging = false;
    m_path.clear();
    m_isMoving = false;
}

// --- UPDATE LOOP (BEYÝN) ---
void Soldier::updateSoldier(float dt, const std::vector<std::shared_ptr<Entity>>& potentialTargets) {
    if (!getIsAlive()) {
        state = SoldierState::Idle;
        return;
    }

    if (attackTimer > 0) attackTimer -= dt;

    // 0. IDLE: Otomatik Hedef Arama
    if (state == SoldierState::Idle) {
        float scanRange = 300.0f; // Görüþ mesafesi
        float bestDist = scanRange;
        std::shared_ptr<Entity> bestTarget = nullptr;

        for (const auto& target : potentialTargets) {
            if (!target || !target->getIsAlive()) continue;
            if (target->getTeam() == this->getTeam()) continue;

            float d = std::sqrt(std::pow(target->getPosition().x - getPosition().x, 2) +
                std::pow(target->getPosition().y - getPosition().y, 2));
            if (d < bestDist) {
                bestDist = d;
                bestTarget = target;
            }
        }
        if (bestTarget) setTarget(bestTarget);
    }

    // 1. CHASING: Kovalama
    else if (state == SoldierState::Chasing) {
        auto target = targetEntity.lock();
        if (!target || !target->getIsAlive()) {
            clearTarget();
            return;
        }

        sf::Vector2f diff = target->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        float targetRadius = target->getBounds().width / 2.0f;
        if (targetRadius < 15.f) targetRadius = 15.f;

        // Menzil Hesabý (Uzakçýlar için radius ekleme)
        float attackReach;
        if (range > 30.0f) attackReach = range + 10.0f; // Okçu/Büyücü
        else attackReach = range + targetRadius + 5.0f; // Barbar

        if (dist <= attackReach) {
            state = SoldierState::Attacking;
            m_path.clear();
            m_isMoving = false;
        }
        else {
            moveTo(target->getPosition());
        }
    }

    // 2. ATTACKING: Saldýrý
    else if (state == SoldierState::Attacking) {
        auto target = targetEntity.lock();
        if (!target || !target->getIsAlive()) {
            clearTarget();
            return;
        }

        sf::Vector2f diff = target->getPosition() - getPosition();
        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        float targetRadius = target->getBounds().width / 2.0f;
        if (targetRadius < 15.f) targetRadius = 15.f;

        // Menzilden Çýkma Hesabý
        float stopDist;
        if (range > 30.0f) stopDist = range + 20.0f;
        else stopDist = range + targetRadius + 20.0f;

        if (dist > stopDist) {
            state = SoldierState::Chasing;
            isCharging = false;
            return;
        }

        // --- BÜYÜCÜ ÖZEL MANTIÐI ---
        if (soldierType == SoldierTypes::Wizard) {
            if (!isCharging) {
                isCharging = true;
                wizardChargeTimer = wizardMaxChargeTime;
            }
            wizardChargeTimer -= dt;

            if (wizardChargeTimer <= 0) {
                isCharging = false;
                // Hasar Vur
                // Eðer binaysa tekli yüksek hasar, askerse alan hasarý
                // (Basitlik için þimdilik direkt vuruyoruz)
                if (CombatSystem::attack(*this, *target)) {
                    // Alan hasarý eklenebilir buraya
                }
            }
        }
        // --- DÝÐERLERÝ ---
        else {
            if (attackTimer <= 0) {
                if (CombatSystem::attack(*this, *target)) {
                    attackTimer = attackInterval;
                }
                else {
                    state = SoldierState::Chasing; // Menzil yetmedi, yürü
                }
            }
        }
    }
}

// --- RENDER (Sadece Karakter) ---
void Soldier::render(sf::RenderWindow& window) {
    if (!getIsAlive()) return;
    Unit::render(window);
}

// --- RENDER EFFECTS (Yýldýrým vb.) ---
void Soldier::renderEffects(sf::RenderWindow& window) {
    if (!getIsAlive()) return;

    // Büyücü Yýldýrým Efekti
    if (soldierType == SoldierTypes::Wizard && isCharging) {
        auto target = targetEntity.lock();
        if (target) {
            sf::Sprite lightning;
            sf::Texture& tex = AssetManager::getTexture("assets/icons/lightning.png");
            lightning.setTexture(tex);

            float iconScale = 0.1f;
            lightning.setScale(iconScale, iconScale);

            // Dolma Efekti
            float percent = 1.0f - (wizardChargeTimer / wizardMaxChargeTime);
            if (percent < 0) percent = 0; if (percent > 1) percent = 1;

            sf::Vector2u texSize = tex.getSize();
            int visibleHeight = static_cast<int>(texSize.y * percent);

            lightning.setTextureRect(sf::IntRect(0, texSize.y - visibleHeight, texSize.x, visibleHeight));

            sf::Vector2f pos = target->getPosition();
            float topPoint = pos.y - 100.0f;
            float visualHeight = visibleHeight * iconScale;
            float totalHeight = texSize.y * iconScale;

            lightning.setPosition(pos.x - (texSize.x * iconScale / 2.0f), topPoint + (totalHeight - visualHeight));

            window.draw(lightning);
        }
    }
}

sf::Texture* Soldier::getIcon() {
    switch (soldierType) {
    case SoldierTypes::Barbarian:
        // Þimdilik birimlerin kendi görsellerini ikon olarak kullanalým
        return &AssetManager::getTexture("assets/units/barbarian.png");
    case SoldierTypes::Archer:
        return &AssetManager::getTexture("assets/units/archer.png");
    case SoldierTypes::Wizard:
        return &AssetManager::getTexture("assets/units/wizard.png");
    default:
        return &AssetManager::getTexture("assets/units/barbarian.png");
    }
}