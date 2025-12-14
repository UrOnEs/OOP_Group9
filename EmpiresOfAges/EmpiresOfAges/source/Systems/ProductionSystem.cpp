#include "Systems/ProductionSystem.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/Soldier.h"
#include <iostream>

bool ProductionSystem::startProduction(Player& player, Barracks& barracks, SoldierTypes unitType) {

    // 1. Kýþla zaten dolu mu?
    if (barracks.getIsProducing()) {
        std::cout << "[INFO] Kisla su an mesgul!\n";
        return false;
    }

    // 2. Nüfus Limiti Kontrolü
    if (player.getUnitCount() >= player.getUnitLimit()) {
        std::cout << "[INFO] Nufus limiti dolu! Ev insa etmelisin.\n";
        return false;
    }

    // 3. Maliyet ve Süre Belirleme (GameRules'dan çekiyoruz)
    int woodCost = 0;
    int foodCost = 0;
    int goldCost = 0;
    float buildTime = 0.0f;

    switch (unitType) {
    case SoldierTypes::Barbarian:
        // Barbar: Yemek + Altýn
        foodCost = GameRules::Cost_Barbarian_Food;
        goldCost = GameRules::Cost_Barbarian_Gold;
        buildTime = GameRules::Time_Build_Soldier;
        break;

    case SoldierTypes::Archer:
        // Okçu: Odun + Altýn
        woodCost = GameRules::Cost_Archer_Wood;
        goldCost = GameRules::Cost_Archer_Gold;
        buildTime = GameRules::Time_Build_Soldier;
        break;

    case SoldierTypes::catapult:
        // Mancýnýk: Çok Odun + Çok Altýn + Uzun Süre
        woodCost = GameRules::Cost_Catapult_Wood;
        goldCost = GameRules::Cost_Catapult_Gold;
        buildTime = GameRules::Time_Build_Soldier * 2.0f; // Mancýnýk 2 kat uzun sürsün
        break;
    }

    // 4. Kaynak Kontrolü
    // Player.cpp getResources sýrasý: { Wood, Gold, Stone, Food }
    // Index: 0=Wood, 1=Gold, 2=Stone, 3=Food
    std::vector<int> resources = player.getResources();

    bool hasWood = resources[0] >= woodCost;
    bool hasGold = resources[1] >= goldCost;
    bool hasFood = resources[3] >= foodCost;

    if (hasWood && hasGold && hasFood) {

        // Ödeme Yap (Negatif ekleme yaparak harcama yapýyoruz)
        if (woodCost > 0) player.addWood(-woodCost);
        if (goldCost > 0) player.addGold(-goldCost);
        if (foodCost > 0) player.addFood(-foodCost);

        // Kýþlada üretimi baþlat
        barracks.startTraining(unitType, buildTime);

        std::cout << "[BASARILI] Uretim basladi. Tur: " << (int)unitType << "\n";
        return true;
    }

    std::cout << "[HATA] Yetersiz Kaynak! (Gereken: "
        << woodCost << " Odun, " << foodCost << " Yemek, " << goldCost << " Altin)\n";
    return false;
}

void ProductionSystem::update(Player& player, Barracks& barracks, float dt) {
    // Üretim yoksa iþlem yapma
    if (!barracks.getIsProducing()) return;

    // Sayacý azalt
    barracks.updateTimer(dt);

    // Süre bitti mi?
    if (barracks.isReady()) {

        // 1. Üretilen türü al
        SoldierTypes type = barracks.finishTraining();

        // 2. Yeni Asker Nesnesini Oluþtur
        std::shared_ptr<Soldier> newSoldier = std::make_shared<Soldier>();

        // --- KRÝTÝK DEÐÝÞÝKLÝK ---
        // Eskisi: newSoldier->soldierType = type; (Sadece etiketi deðiþtiriyordu)
        // Yenisi: setType fonksiyonu can, hasar ve hýzý da ayarlar!
        newSoldier->setType(type);

        // 3. Konumlandýrma (Kýþlanýn kapýsýnda doðsun)
        sf::Vector2f spawnPos = barracks.getPosition();
        // Binanýn biraz saðýna ve aþaðýsýna koy (TileSize kadar)
        spawnPos.x += GameRules::TileSize * 1.5f;
        spawnPos.y += GameRules::TileSize * 1.5f;
        newSoldier->setPosition(spawnPos);

        // 4. Texture Atama (Geçici olarak Player üzerinden veya AssetManager'dan)
        // Eðer Player'da texturelar varsa: 
        // newSoldier->setTexture(player.playerResources.getTexture("Soldier"));

        // Veya texture yoksa belli olsun diye renk verelim (Test için)
        if (type == SoldierTypes::Barbarian) newSoldier->getModel().setFillColor(sf::Color::Red);
        else if (type == SoldierTypes::Archer) newSoldier->getModel().setFillColor(sf::Color::Green);
        else newSoldier->getModel().setFillColor(sf::Color::Magenta);

        // 5. Oyuncunun ordusuna ekle
        player.addEntity(newSoldier);

        std::cout << "[INFO] Asker sahaya indi! Stats: " << newSoldier->stats() << "\n";
    }
}