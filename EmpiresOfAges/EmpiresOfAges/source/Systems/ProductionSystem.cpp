#include "Systems/ProductionSystem.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/Soldier.h"
#include <iostream>
#include <Entity System/Entity Type/Villager.h>

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

    // GameRules'dan paketi (struct) istiyoruz
    GameRules::Cost cost = GameRules::getUnitCost(unitType);

    // Deðerleri yerel deðiþkenlere alalým ki aþaðýdaki kodlar bozulmasýn
    int woodCost = cost.wood;
    int foodCost = cost.food;
    int goldCost = cost.gold;

    float buildTime = GameRules::Time_Build_Soldier;

    // 4. Kaynak Kontrolü (Player kaynaklarýný kontrol et)
    std::vector<int> resources = player.getResources(); // { Wood, Gold, Stone, Food }

    bool hasWood = resources[0] >= woodCost;
    bool hasGold = resources[1] >= goldCost;
    bool hasFood = resources[3] >= foodCost;

    if (hasWood && hasGold && hasFood) {

        // Ödeme Yap
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

bool ProductionSystem::startVillagerProduction(Player& player, TownCenter& tc) {
    if (tc.isReady()) { // isProducing kontrolü yerine isReady/getIsProducing kullanabilirsin
        // TownCenter.h'da getIsProducing yoksa, doðrudan tc deðiþkenlerine eriþmek gerekebilir 
        // veya TownCenter.h'a getter eklemelisin.
        // Þimdilik TownCenter kodunda isProducing private ama startProduction kontrol ediyor.
    }

    // TownCenter.h'a "getIsProducing()" eklediðini varsayýyorum.
    // Eðer yoksa TownCenter.h dosyasýný güncelle: bool getIsProducing() const { return isProducing; }

    if (player.getUnitCount() >= player.getUnitLimit()) {
        std::cout << "[INFO] Nufus dolu!\n";
        return false;
    }

    GameRules::Cost cost = GameRules::Cost_Villager; // {0, 50, 0, 0}

    if (player.getResources()[3] >= cost.food) { // Yemek kontrolü
        player.addFood(-cost.food);
        tc.startProduction(); // Bu fonksiyon TownCenter.h'da zaten var
        std::cout << "[BASARILI] Koylu uretimi basladi.\n";
        return true;
    }

    std::cout << "[HATA] Yetersiz Yemek! (50 Yemek Gerekli)\n";
    return false;
}

void ProductionSystem::updateTC(Player& player, TownCenter& tc, float dt) {
    // TownCenter içindeki timer'ý güncelle
    tc.updateTimer(dt);

    if (tc.isReady()) {
        tc.finishProduction();

        // Köylüyü Oluþtur
        std::shared_ptr<Villager> newVillager = std::make_shared<Villager>();

        // Konum: Binanýn önüne koy
        sf::Vector2f spawnPos = tc.getPosition();
        spawnPos.y += 150.0f; // Biraz aþaðýsý
        newVillager->setPosition(spawnPos);

        player.addEntity(newVillager);
        std::cout << "[INFO] Yeni koylu dogdu!\n";
    }
}