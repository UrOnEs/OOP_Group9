#pragma once

#include <SFML/System/Vector2.hpp>

namespace GameRules {

    // ==========================================
    // 1. GENEL AYARLAR (General Settings)
    // ==========================================
    constexpr int TileSize = 32;          // Harita kare boyutu (Piksel)
    constexpr int MaxPopulation = 200;    // Maksimum nüfus limiti (AoE standardý)
    constexpr float GameSpeed = 1.0f;     // Oyun hýzý çarpaný

    // ==========================================
    // 2. MALÝYETLER (Costs)
    // ==========================================

    // -- BÝNALAR --
    // Ev ucuz olmalý ki nüfus artýrýlabilsin
    constexpr int Cost_House_Wood = 30;

    // Üretim binalarý orta maliyetli
    constexpr int Cost_Barracks_Wood = 175;
    constexpr int Cost_Farm_Wood = 60; // Tarlalar tükeniyorsa ucuz olmalý

    // Madenler ve Kamp
    constexpr int Cost_LumberCamp_Wood = 100;
    constexpr int Cost_MiningCamp_Wood = 100;

    // Ana Merkez (Town Center) - Çok pahalý
    constexpr int Cost_TownCenter_Wood = 275;
    constexpr int Cost_TownCenter_Stone = 100;

    // -- BÝRÝMLER (UNITS) --
    // Köylü: Sadece yemekle üretilir (Genelde)
    constexpr int Cost_Villager_Food = 50;

    // Barbar (Yakýn dövüþ): Yemek + Altýn (Güçlü birim)
    constexpr int Cost_Barbarian_Food = 60;
    constexpr int Cost_Barbarian_Gold = 20;

    // Okçu (Menzilli): Odun + Altýn
    constexpr int Cost_Archer_Wood = 25;
    constexpr int Cost_Archer_Gold = 45;

    // Mancýnýk (Kuþatma): Çok Odun + Çok Altýn
    constexpr int Cost_Catapult_Wood = 160;
    constexpr int Cost_Catapult_Gold = 135;

    // ==========================================
    // 3. SAÐLIK DEÐERLERÝ (Health Points)
    // ==========================================

    // Binalar
    constexpr float BuildingHealth = 250.f;
    constexpr float HP_House = 250.f;        // Çabuk yýkýlýr
    constexpr float HP_Barracks = 1200.f;    // Askeri bina saðlam olmalý
    constexpr float HP_Farm = 100.f;         // Tarlayý yakmak kolaydýr
    constexpr float HP_TownCenter = 2500.f;  // Ana bina yýkýlmasý en zor olandýr
    constexpr float HP_Wall = 1500.f;        // Duvarlar serttir

    // Birimler
    constexpr float HP_Villager = 25.f;      // Köylüler çok kýrýlgandýr
    constexpr float HP_Barbarian = 100.f;    // Tank görevi görür
    constexpr float HP_Archer = 35.f;        // Okçular yakalanýrsa hemen ölür
    constexpr float HP_Catapult = 75.f;      // Makine olduðu için orta halli ama savunmasýz

    // ==========================================
    // 4. HASAR GÜCÜ (Damage)
    // ==========================================

    constexpr float Dmg_Villager = 3.f;      // Sadece kendini savunur
    constexpr float Dmg_Barbarian = 10.f;    // Standart vuruþ
    constexpr float Dmg_Archer = 6.f;        // Uzaktan vurduðu için hasarý az (Denge)
    constexpr float Dmg_Catapult = 50.f;     // Binalara ve alanlara çok vurur

    // ==========================================
    // 5. HAREKET HIZLARI (Speed)
    // ==========================================
    // Not: DeltaTime ile çarpýlacaðý için bu deðerler piksel/saniye cinsinden düþünülmeli

    constexpr float Speed_Villager = 40.f;
    constexpr float Speed_Barbarian = 50.f;  // En hýzlýsý (piyade hücumu)
    constexpr float Speed_Archer = 45.f;     // "Vur-Kaç" yapabilsin diye fena deðil
    constexpr float Speed_Catapult = 20.f;   // Çok yavaþ

    // ==========================================
    // 6. MENZÝL (Range) - (Tile cinsinden deðil Pixel cinsinden olabilir)
    // ==========================================

    constexpr float Range_Melee = 20.f;      // Dibine girmesi lazým (Kýlýç boyu)
    constexpr float Range_Archer = 200.f;    // Ekranda iyi bir mesafe
    constexpr float Range_Catapult = 350.f;  // Okçudan daha uzaða atmalý

    constexpr float Range_Sight_Normal = 300.f; // Görüþ mesafesi

    // ==========================================
    // 7. SÜRELER (Timers)
    // ==========================================

    constexpr float Time_Build_Villager = 5.0f; // 5 saniyede bir köylü (Test için hýzlý)
    constexpr float Time_Build_Soldier = 10.0f;
    constexpr float Time_Harvest_Tick = 1.0f;   // Madenci ne sýklýkla kaynak toplasýn?

    // ==========================================
    // 8. OYUN ALANI (Spaces)
    // ==========================================
    // Binalarýn kapladýðý alan (Çarpýþma kutusu için)
    const sf::Vector2f Size_Unit = sf::Vector2f(20.f, 20.f);
    const sf::Vector2f Size_House = sf::Vector2f(64.f, 64.f); // 2x2 Tile
    const sf::Vector2f Size_Barracks = sf::Vector2f(96.f, 96.f); // 3x3 Tile
}