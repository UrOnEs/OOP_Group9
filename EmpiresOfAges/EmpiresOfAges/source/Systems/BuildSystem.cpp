#include "Systems/BuildSystem.h"
#include "Game/Player.h"
#include "Game/GameRules.h"

bool BuildSystem::build(Player& player, Villager& worker, Building& building, const sf::Vector2f& pos) {

    // 1. Maliyet Kontrolü (Basitçe Wood varsayýyorum, tipe göre deðiþebilir)
    int cost = GameRules::SoldierCost_Wood; // Örnek, bina cost'u GameRules'a eklenebilir

    // Oyuncunun kaynaklarýný kontrol etmek için Player.h'a getResources fonksiyonuna index ile eriþiyoruz
    // (Daha temiz olmasý için Player'a 'getWood()' eklemek iyi olabilir ama þimdilik vector ile:)
    if (player.getResources()[0] < cost) { // 0: Wood
        return false;
    }

    // 2. Alan Kontrolü
    if (!checkSpace(building, pos)) {
        return false;
    }

    // 3. Ýnþaat Ýþlemi
    // Kaynaðý düþ (Player'ýn resources'ý private olduðu için player.h'a harcama fonksiyonu eklemek gerekebilir)
    // Þimdilik Resource Manager'a eriþimimiz olmadýðý için bu kýsmý Player sýnýfýna ekleyeceðin bir metotla yapmalýsýn.
    // Örn: player.spendWood(cost);

    building.setPosition(pos);
    return true;
}

bool BuildSystem::checkSpace(Building& building, const sf::Vector2f& pos) {
    // Harita/Tile sistemi gelince burasý dolacak. Þimdilik her yer inþa edilebilir.
    return true;
}