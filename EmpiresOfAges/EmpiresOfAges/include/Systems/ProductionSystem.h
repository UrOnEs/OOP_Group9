#pragma once

#include "Entity System/Entity Type/Barracks.h" // Sadece Barracks ile çalýþýr
#include "Entity System/Entity Type/types.h"
#include "Game/Player.h"

class ProductionSystem {
public:
    // Oyuncu butona bastýðýnda çaðrýlýr
    // True dönerse: Para düþtü, üretim baþladý. False: Yetersiz bakiye/yer/meþgul.
    static bool startProduction(Player& player, Barracks& barracks, SoldierTypes unitType);

    // Update döngüsünde çaðrýlýr
    // Süre bittiðinde askeri oluþturup Player'ýn listesine ekler
    static void update(Player& player, Barracks& barracks, float dt);
};