#pragma once
#include <SFML/System/Vector2.hpp>
#include "Entity System/Entity Type/types.h" 

namespace GameRules {

    static bool DebugMode;

    // ==========================================
    // 1. GENEL AYARLAR & HARÝTA
    // ==========================================
    // --- GÜNCELLEME: Harita Boyutu 4 Katýna Çýkarýldý (200x200) ---
    constexpr int MapWidth = 200;
    constexpr int MapHeight = 200;
    constexpr int TileSize = 32;
    constexpr float UnitRadius = 8.0f;
    constexpr int MaxPopulation = 200;
    constexpr float GameSpeed = 1.0f;

    // ==========================================
    //               SES AYARLARI    
    // ==========================================
    constexpr float BackgroundMusicVolume = 50.0f;

    // ==========================================
    // 2. MALÝYET SÝSTEMÝ
    // ==========================================
    struct Cost {
        int wood = 0;
        int food = 0;
        int gold = 0;
        int stone = 0;
    };

    constexpr Cost getBuildingCost(BuildTypes type) {
        switch (type) {
        case BuildTypes::House:      return { 30, 0, 0, 0 };
        case BuildTypes::Farm:       return { 60, 0, 0, 0 };
        case BuildTypes::Barrack:    return { 175, 0, 0, 0 };
        case BuildTypes::TownCenter: return { 400, 0, 0, 200 }; // Ana Bina Pahalý
        case BuildTypes::WoodPlace:  return { 100, 0, 0, 0 };
        case BuildTypes::StoneMine:  return { 100, 0, 0, 0 };
        case BuildTypes::GoldMine:   return { 100, 0, 0, 0 };
        default:                     return { 0, 0, 0, 0 };
        }
    }

    constexpr Cost getUnitCost(SoldierTypes type) {
        switch (type) {
        case SoldierTypes::Barbarian: return { 0, 60, 20, 0 };
        case SoldierTypes::Archer:    return { 25, 20, 45, 0 };
        case SoldierTypes::Wizard:    return { 160, 60, 135, 0 };
        default:                      return { 0, 0, 0, 0 };
        }
    }

    constexpr Cost Cost_Villager = { 0, 50, 0, 0 };

    // ==========================================
     // 3. SAÐLIK DEÐERLERÝ
     // ==========================================
    constexpr float BuildingHealth = 250.f;
    constexpr float HP_House = 250.f;
    constexpr float HP_Barracks = 1200.f;
    constexpr float HP_Farm = 100.f;
    constexpr float HP_TownCenter = 2500.f;
    constexpr float HP_Wall = 1500.f;
    constexpr float HP_Tree = 50.f;
    constexpr float HP_Stone = 500.f; // <-- YENÝ: Taþýn caný (Kýrýlmasý daha zor olsun)
    constexpr float HP_Gold = 300.f; // <-- YENÝ: Altýn caný

    constexpr float HP_Villager = 25.f;
    constexpr float HP_Barbarian = 100.f;
    constexpr float HP_Archer = 35.f;
    constexpr float HP_Wizard = 75.f;

    // ==========================================
    // 4. HASAR, HIZ VE DÝÐERLERÝ
    // ==========================================
    constexpr float Dmg_Villager = 5.f;
    constexpr float Dmg_Barbarian = 15.f;
    constexpr float Dmg_Archer = 20.f;
    constexpr float Dmg_Wizard = 200.f;

    constexpr float AttackSpeed_Villager = 2.f;
    constexpr float AttackSpeed_Barbarian = 1.f;
    constexpr float AttackSpeed_Archer = 3.f;
    constexpr float AttackSpeed_Wizard = 10.f;

    constexpr float Speed_Villager = 100.f;
    constexpr float Speed_Barbarian = 150.f;
    constexpr float Speed_Archer = 145.f;
    constexpr float Speed_Wizard = 120.f;

    constexpr float Range_Melee = 20.f;
    constexpr float Range_Archer = 200.f;
    constexpr float Range_Wizard = 350.f;
    constexpr float Range_Sight_Normal = 300.f;

    constexpr float Time_Build_Villager = 5.0f;
    constexpr float Time_Build_Soldier = 10.0f;
    constexpr float Time_Harvest_Tick = 1.0f;

    // --- GÜNCELLENEN GÖRÜÞ MENZÝLLERÝ (FOG OF WAR) ---
    constexpr float Range_Sight_Unit = 250.f;      // Askerler ve Köylüler için standart görüþ
    constexpr float Range_Sight_Building = 350.f;  // Binalar için geniþ görüþ
    constexpr float Range_Sight_TownCenter = 600.f;// Ana Bina için ekstra geniþ görüþ (Opsiyonel)


    // ==========================================
    // 9. OYUN ALANI (BÝNA BOYUTLARI)
    // ==========================================
    const sf::Vector2f Size_Unit = sf::Vector2f(10.f, 10.f);
    const sf::Vector2f Size_House = sf::Vector2f(64.f, 64.f);     // 1x1
    const sf::Vector2f Size_Barracks = sf::Vector2f(128.f, 128.f);// 2x2
    const sf::Vector2f Size_Farm = sf::Vector2f(128.f, 128.f);    // 2x2 (Mill)
    const sf::Vector2f Size_TownCenter = sf::Vector2f(192.f, 192.f); // 3x3


    constexpr int Resources_Per_Tree = 500;
    constexpr int Resources_Per_Stone = 300;
    constexpr int Resources_Per_Gold = 500;  // <-- YENÝ: Bir altýnda ne kadar kaynak var?
    constexpr int Wood_Per_Tick = 10;
    constexpr int Stone_Per_Tick = 10;
    constexpr int Gold_Per_Tick = 10;        // <-- YENÝ: Toplama hýzý

    constexpr int Villager_MaxCargo = 50; 
    constexpr float Villager_BuildSpeed = 100.0f;

    const sf::Vector2i SpawnPoints[] = {
        {10, 10},     // Oyuncu 0:
        {180, 180},   // Oyuncu 1:
        {180, 10},    // Oyuncu 2:
        {10, 180}     // Oyuncu 3:
    };
}
