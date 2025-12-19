#pragma once

#include <SFML/System/Vector2.hpp>
#include "Entity System/Entity Type/types.h" 

namespace GameRules {

    // ==========================================
    // 1. GENEL AYARLAR & HARÝTA
    // ==========================================
    constexpr int MapWidth = 50;
    constexpr int MapHeight = 50;
    constexpr int TileSize = 64;

    // BÝRÝM BOYUTLANDIRMA (YENÝ)
    // Askerlerin yarýçapý. (Örn: 15.0f yaparsan çapý 30 piksel olur)
    constexpr float UnitRadius = 8.0f;

    constexpr int MaxPopulation = 200;
    constexpr float GameSpeed = 1.0f;

    // ==========================================
    // 2. MALÝYET SÝSTEMÝ (STRUCT YAPISI)
    // ==========================================

    struct Cost {
        int wood = 0;
        int food = 0;
        int gold = 0;
        int stone = 0;
    };

    // Binalarýn Maliyeti
    constexpr Cost getBuildingCost(BuildTypes type) {
        switch (type) {
        case BuildTypes::House:      return { 30, 0, 0, 0 };   // 30 Odun
        case BuildTypes::Farm:       return { 60, 0, 0, 0 };   // 60 Odun
        case BuildTypes::Barrack:    return { 175, 0, 0, 0 };  // 175 Odun
        case BuildTypes::WoodPlace:  return { 100, 0, 0, 0 };  // 100 Odun
        case BuildTypes::StoneMine:  return { 100, 0, 0, 0 };  // 100 Odun
        case BuildTypes::GoldMine:   return { 100, 0, 0, 0 };  // 100 Odun
        default:                     return { 0, 0, 0, 0 };
        }
    }

    // Askerlerin Maliyeti
    constexpr Cost getUnitCost(SoldierTypes type) {
        switch (type) {
        case SoldierTypes::Barbarian: return { 0, 60, 20, 0 };   // 60 Yemek, 20 Altýn
        case SoldierTypes::Archer:    return { 25, 20, 45, 0 };  // 25 Odun, 20 Yemek, 45 Altýn
        case SoldierTypes::Wizard:    return { 160, 60, 135, 0 };// 160 Odun, 60 Yemek, 135 Altýn
        default:                      return { 0, 0, 0, 0 };
        }
    }

    // Köylü Maliyeti (Enum dýþýnda olduðu için manuel)
    constexpr Cost Cost_Villager = { 0, 50, 0, 0 }; // 50 Yemek

    // ==========================================
    // 3. SAÐLIK DEÐERLERÝ
    // ==========================================
    constexpr float BuildingHealth = 250.f;
    constexpr float HP_House = 250.f;
    constexpr float HP_Barracks = 1200.f;
    constexpr float HP_Farm = 100.f;
    constexpr float HP_TownCenter = 2500.f;
    constexpr float HP_Wall = 1500.f;

    constexpr float HP_Villager = 25.f;
    constexpr float HP_Barbarian = 100.f;
    constexpr float HP_Archer = 35.f;
    constexpr float HP_Wizard = 75.f;

    // ==========================================
    // 4. HASAR GÜCÜ (Damage)
    // ==========================================
    constexpr float Dmg_Villager = 5.f;
    constexpr float Dmg_Barbarian = 15.f;
    constexpr float Dmg_Archer = 20.f;
    constexpr float Dmg_Wizard = 200.f;

    // ==========================================
    // 5. SALDIRI HIZI (AttackSpeed_)
    // ==========================================
    constexpr float AttackSpeed_Villager = 2.f;
    constexpr float AttackSpeed_Barbarian = 1.f;
    constexpr float AttackSpeed_Archer = 3.f;
    constexpr float AttackSpeed_Wizard = 10.f;

    // ==========================================
    // 6. HAREKET HIZLARI
    // ==========================================
    constexpr float Speed_Villager = 100.f;
    constexpr float Speed_Barbarian = 150.f;
    constexpr float Speed_Archer = 45.f;
    constexpr float Speed_Wizard = 20.f;

    // ==========================================
    // 7. MENZÝL
    // ==========================================
    constexpr float Range_Melee = 20.f;
    constexpr float Range_Archer = 200.f;
    constexpr float Range_Wizard = 350.f;
    constexpr float Range_Sight_Normal = 300.f;

    // ==========================================
    // 8. SÜRELER
    // ==========================================
    constexpr float Time_Build_Villager = 5.0f;
    constexpr float Time_Build_Soldier = 10.0f;
    constexpr float Time_Harvest_Tick = 1.0f;

    // ==========================================
    // 9. OYUN ALANI
    // ==========================================
    const sf::Vector2f Size_Unit = sf::Vector2f(10.f, 10.f);

    // EV: 1x1 Kare (64x64)
    const sf::Vector2f Size_House = sf::Vector2f(64.f, 64.f);

    // KIÞLA: 2x2 Kare (128x128)
    const sf::Vector2f Size_Barracks = sf::Vector2f(128.f, 128.f);

    // ORMAN / AÐAÇ AYARLARI
    constexpr float HP_Tree = 50.f;        // Aðacýn caný (Kesilince deðil, saldýrýlýnca yýkýlmasý için) %90 kaldýrýrýk
    constexpr int Resources_Per_Tree = 100;// Bir aðaçtan kaç odun çýkar?
    constexpr int Wood_Per_Tick = 10;      // Her seferinde kaç odun toplansýn?
}