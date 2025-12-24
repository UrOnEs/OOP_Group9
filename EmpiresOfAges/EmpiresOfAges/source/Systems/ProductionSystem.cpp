#include "Systems/ProductionSystem.h"
#include "Game/GameRules.h"
#include "Entity System/Entity Type/Soldier.h"
#include "Entity System/Entity Type/Villager.h"
#include "Entity System/Entity Type/TownCenter.h"
#include <iostream>

// ======================================================================================
//                                  ASKER ÜRETÝMÝ (BARRACKS)
// ======================================================================================

bool ProductionSystem::startProduction(Player& player, Barracks& barracks, SoldierTypes unitType) {

    // 1. Nüfus Limiti Kontrolü
    // (Ýdealde bekleyenler de sayýlmalý ama þimdilik mevcut askerlere bakýyoruz)
    if (player.getUnitCount() >= player.getUnitLimit()) {
        std::cout << "[INFO] Nufus limiti dolu! Ev insa etmelisin.\n";
        return false;
    }

    // 2. Maliyet Hesaplama
    GameRules::Cost cost = GameRules::getUnitCost(unitType);

    // 3. Kaynak Kontrolü
    std::vector<int> resources = player.getResources(); // { Wood, Gold, Stone, Food }

    // Wood=0, Gold=1, Stone=2, Food=3 (ResourceManager sýrasýna göre)
    bool hasWood = resources[0] >= cost.wood;
    bool hasGold = resources[1] >= cost.gold;
    bool hasFood = resources[3] >= cost.food;

    if (hasWood && hasGold && hasFood) {

        // 4. Ödeme Yap (Kaynaklarý Düþ)
        if (cost.wood > 0) player.addWood(-cost.wood);
        if (cost.gold > 0) player.addGold(-cost.gold);
        if (cost.food > 0) player.addFood(-cost.food);

        // 5. Üretimi Baþlat veya Sýraya Ekle
        // Barracks sýnýfý, eðer meþgulse bu isteði otomatik olarak kuyruða (queue) atacak þekilde güncellendi.
        float buildTime = GameRules::Time_Build_Soldier;
        barracks.startTraining(unitType, buildTime);

        std::cout << "[BASARILI] Asker uretim emri verildi. (Tur: " << (int)unitType << ")\n";
        return true;
    }

    std::cout << "[HATA] Yetersiz Kaynak! (Gereken: "
        << cost.wood << " Odun, " << cost.food << " Yemek, " << cost.gold << " Altin)\n";
    return false;
}

void ProductionSystem::update(Player& player, Barracks& barracks, float dt) {
    // Üretim yoksa iþlem yapma
    if (!barracks.getIsProducing()) return;

    // Sayacý azalt
    barracks.updateTimer(dt);

    // Süre bitti mi?
    if (barracks.isReady()) {

        // 1. Üretilen türü al (Bina sýnýfý bu aþamada sýradaki üretime otomatik geçer)
        SoldierTypes type = barracks.finishTraining();

        // 2. Yeni Asker Nesnesini Oluþtur
        std::shared_ptr<Soldier> newSoldier = std::make_shared<Soldier>();

        // 3. Türü ve Özellikleri Ata
        // (Soldier::setType içinde texture atamasý ve stat ayarlarý yapýlýyor)
        newSoldier->setType(type);

        // 4. Konumlandýrma (Kýþlanýn kapýsýnda doðsun)
        sf::Vector2f spawnPos = barracks.getPosition();
        // Binanýn biraz saðýna ve aþaðýsýna koy (TileSize kadar)
        spawnPos.x += GameRules::TileSize * 2.5f;
        spawnPos.y += GameRules::TileSize * 2.5f;
        newSoldier->setPosition(spawnPos);

        // 5. Oyuncunun ordusuna ekle
        player.addEntity(newSoldier);

        std::cout << "[INFO] Asker egitimi tamamlandi!\n";
    }
}

// ======================================================================================
//                                  KÖYLÜ ÜRETÝMÝ (TOWN CENTER)
// ======================================================================================

bool ProductionSystem::startVillagerProduction(Player& player, TownCenter& tc) {

    // 1. Nüfus Kontrolü
    if (player.getUnitCount() >= player.getUnitLimit()) {
        std::cout << "[INFO] Nufus dolu!\n";
        return false;
    }

    // 2. Maliyet
    GameRules::Cost cost = GameRules::Cost_Villager; // {0, 50, 0, 0}

    // 3. Kaynak Kontrolü ve Ödeme
    if (player.getResources()[3] >= cost.food) { // Index 3 = Food

        // Parayý Peþin Al
        player.addFood(-cost.food);

        // 4. Üretimi Baþlat (veya Sýraya Ekle)
        // TownCenter sýnýfý meþgulse sayacý (queuedVillagers) artýracak.
        tc.startProduction();

        std::cout << "[BASARILI] Koylu uretim emri alindi.\n";
        return true;
    }

    std::cout << "[HATA] Yetersiz Yemek! (50 Yemek Gerekli)\n";
    return false;
}

void ProductionSystem::updateTC(Player& player, TownCenter& tc, float dt) {
    // TownCenter içindeki timer'ý güncelle
    // (Eðer kuyrukta kimse yoksa ve üretim bittiyse burasý bir þey yapmaz)
    tc.updateTimer(dt);

    if (tc.isReady()) {
        // 1. Üretimi bitir (Sýradaki varsa onu baþlatýr)
        tc.finishProduction();

        // 2. Köylüyü Oluþtur
        std::shared_ptr<Villager> newVillager = std::make_shared<Villager>();

        // 3. Konum: Binanýn önüne koy
        sf::Vector2f spawnPos = tc.getPosition();
        spawnPos.y += 150.0f; // Biraz aþaðýsý
        newVillager->setPosition(spawnPos);

        // 4. Oyuncuya Ekle
        player.addEntity(newVillager);

        std::cout << "[INFO] Yeni koylu isbasi yapti!\n";
    }
}