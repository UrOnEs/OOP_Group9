#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/Building.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/ResourceGenerator.h"
#include <iostream>
#include <cmath>

int Villager::IDcounter = 0;
int Villager::counter = 0;

Villager::Villager() : Unit() {
    this->health = GameRules::HP_Villager;
    this->damage = GameRules::Dmg_Villager;
    this->travelSpeed = GameRules::Speed_Villager;
    this->range = GameRules::Range_Melee;

    this->isAlive = true;
    this->isSelected = false;
    this->entityID = ++IDcounter;

    counter++;
}

Villager::~Villager() {
    counter--;
}

std::string Villager::stats() {
    return "Villager HP: " + std::to_string((int)health);
}

// =========================================================
//               HASAT VE HAREKET MANTIÐI 
// =========================================================

void Villager::startHarvesting(std::shared_ptr<ResourceGenerator> resource) {
    if (!resource) return;

    // weak_ptr'a shared_ptr atanabilir (otomatik dönüþüm)
    targetResource = resource;
    isHarvesting = true;

    moveTo(resource->getPosition());
}

void Villager::stopHarvesting() {
    // Önce kilitliyoruz, eðer hala varsa iþlem yapýyoruz
    if (isHarvesting) {
        if (auto res = targetResource.lock()) {
            res->releaseWorker();
        }
    }
    isHarvesting = false;
    targetResource.reset(); // Baðý kopar
}

void Villager::checkTargetStatus() {
    if (isHarvesting) {
        // Hedefe eriþmeye çalýþ
        if (auto res = targetResource.lock()) {
            if (!res->getIsAlive()) {
                stopHarvesting();
            }
        }
        else {
            // Eðer lock() boþ döndüyse, aðaç silinmiþ demektir.
            stopHarvesting();
        }
    }
}

void Villager::updateHarvesting(const std::vector<std::shared_ptr<Building>>& buildings) {
    if (!isHarvesting) return;

    auto res = targetResource.lock();

    // 1. Durum: Hedef Kaynak Yok Olduysa (Kesildi/Yýkýldý)
    if (!res || !res->getIsAlive()) {
        // Önce mevcut iþlemi temizle
        stopHarvesting();

        // SONRA YENÝSÝNÝ ARA (Otomasyon Kýsmý)
        findNearestResource(buildings);
        return;
    }

    // 2. Durum: Hedef Hala Var, Oraya Git veya Kes
    sf::Vector2f diff = res->getPosition() - getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    if (dist <= 50.0f) {
        if (!res->isWorking()) {
            if (res->garrisonWorker()) {
                // Aðaca girince yürüme dursun
                m_path.clear();
                m_isMoving = false;
            }
        }
    }
}

void Villager::findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings) {
    float minDistance = 500.0f;
    std::shared_ptr<ResourceGenerator> closest = nullptr;

    // Mevcut hedefi al (varsa)
    auto currentTarget = targetResource.lock();

    for (const auto& building : buildings) {
        if (building) {
            if (building->buildingType == BuildTypes::Tree) { // Sadece aðaç ara
                if (auto res = std::dynamic_pointer_cast<ResourceGenerator>(building)) {

                    // Þu anki hedefe tekrar gitme ve ölüleri sayma
                    if (res->getIsAlive() && res != currentTarget) {

                        sf::Vector2f diff = res->getPosition() - getPosition();
                        float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                        if (dist < minDistance) {
                            minDistance = dist;
                            closest = res;
                        }
                    }
                }
            }
        }
    }

    if (closest) {
        std::cout << "[Villager] Yeni agac bulundu! Oraya gidiliyor...\n";
        startHarvesting(closest);
    }
    else {
        std::cout << "[Villager] Yakinda baska agac yok. Bekliyor.\n";
        stopHarvesting();
    }
}