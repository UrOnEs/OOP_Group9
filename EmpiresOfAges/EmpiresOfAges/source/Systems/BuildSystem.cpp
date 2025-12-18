#include "Systems/BuildSystem.h"
#include "Game/Player.h"
#include "Game/GameRules.h" // Yeni GameRules yapýsýný kullanacaðýz
#include "Map/MapManager.h" // MapManager'a eriþim gerekebilir

// Not: Fonksiyon imzasýný Player.h veya BuildSystem.h'a uygun þekilde çaðýrdýðýndan emin ol.
// Þu an BuildSystem.h'da: bool build(Player&, Villager&, Building&, pos) var.
// Ancak biz Game.cpp'de daha modern bir yapý kullanýyoruz. 
// Þimdilik sadece hatayý gidermek için bu dosyadaki eski kodlarý güncelliyorum:

bool BuildSystem::build(Player& player, Villager& worker, Building& building, const sf::Vector2f& pos) {

    // --- YENÝ SÝSTEM: MALÝYET KONTROLÜ ---
    // 1. Binanýn maliyetini öðren
    GameRules::Cost cost = GameRules::getBuildingCost(building.buildingType);

    // 2. Oyuncunun parasý yetiyor mu? (ResourceManager'a yazdýðýmýz yeni metot)
    // Not: Player.h içinde 'playerResources' private ise getResources() ile eriþmek zor olabilir.
    // Bu yüzden Player.h içinde 'ResourceManager' public olmalý veya getter referans döndürmeli.
    // Þimdilik varsayým: Player.h içinde þu fonksiyon var: ResourceManager& getResourceManager();

    // Geçici olarak vector kontrolü (Eðer ResourceManager'a eriþemiyorsan):
    std::vector<int> currentRes = player.getResources();
    if (currentRes[0] < cost.wood || currentRes[1] < cost.gold) {
        return false; // Yetersiz kaynak
    }

    // 3. Alan Kontrolü
    if (!checkSpace(building, pos)) {
        return false;
    }

    // 4. Ýnþaat Ýþlemi
    // Kaynaðý düþ
    player.addWood(-cost.wood);
    player.addGold(-cost.gold);
    player.addFood(-cost.food);
    player.addStone(-cost.stone);

    building.setPosition(pos);
    return true;
}

bool BuildSystem::checkSpace(Building& building, const sf::Vector2f& pos) {
    // Harita/Tile sistemi gelince burasý MapManager üzerinden kontrol edilecek
    return true;
}