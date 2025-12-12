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

    // 2. Nüfus Limiti Kontrolü (Player.h'a getUnitCount eklediðini varsayýyorum)
    // Eðer yoksa: player.getEntities().size() kullanýlabilir.
    if (player.getUnitCount() >= player.getUnitLimit()) {
        std::cout << "[INFO] Nufus limiti dolu! Ev insa etmelisin.\n";
        return false;
    }

    // 3. Maliyet ve Süre Belirleme
    int woodCost = 0;
    int goldCost = 0;
    float buildTime = 0.0f;

    switch (unitType) {
    case SoldierTypes::Barbarian:
        woodCost = GameRules::SoldierCost_Wood;
        buildTime = 2.0f; // 2 saniye
        break;
    case SoldierTypes::Archer:
        woodCost = 150; // GameRules'a ekleyebilirsin: ArcherCost_Wood
        goldCost = 50;
        buildTime = 3.0f;
        break;
    case SoldierTypes::catapult:
        woodCost = 300;
        goldCost = 100;
        buildTime = 5.0f;
        break;
    }

    // 4. Kaynak Kontrolü
    // Player kaynaklarýný vector olarak çekiyoruz: [0] Wood, [1] Gold
    std::vector<int> resources = player.getResources();

    if (resources[0] >= woodCost && resources[1] >= goldCost) {

        // Ödeme Yap
        player.addWood(-woodCost);
        player.addGold(-goldCost);

        // Kýþlada üretimi baþlat
        barracks.startTraining(unitType, buildTime);

        std::cout << "[BASARILI] Uretim basladi. Maliyet: " << woodCost << " Odun, " << goldCost << " Altin.\n";
        return true;
    }

    std::cout << "[HATA] Yetersiz Kaynak!\n";
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
        newSoldier->soldierType = type;

        // 3. Konumlandýrma (Kýþlanýn kapýsýnda doðsun)
        // Binanýn pozisyonunu alýp biraz sað-altýna koyuyoruz ki binanýn içinde kalmasýn
        sf::Vector2f spawnPos = barracks.getPosition();
        spawnPos.x += 50.0f;
        spawnPos.y += 50.0f;
        newSoldier->setPosition(spawnPos);

        // 4. Oyuncuya Özel Temayý Giydir (Texture)
        // Player sýnýfýndaki texture pointerlarýný kullanarak askere giydiriyoruz.
        // NOT: Player.h'a "assignThemeTo(Entity&)" fonksiyonu eklersen daha temiz olur.
        // Þimdilik varsayýlan texture veya Player içindeki bir helper ile yapýlabilir.

        // 5. Oyuncunun ordusuna ekle
        // Player.h içinde 'addEntity' fonksiyonu olmasý lazým!
        player.addEntity(newSoldier);

        std::cout << "[INFO] Asker sahaya indi!\n";
    }
}