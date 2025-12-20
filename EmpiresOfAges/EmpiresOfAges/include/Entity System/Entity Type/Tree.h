#pragma once
#include "ResourceGenerator.h"
#include "Game/GameRules.h"

class Tree : public ResourceGenerator {
public:
    Tree() {
        buildingType = BuildTypes::Tree;
        health = GameRules::HP_Tree;

        interval = GameRules::Time_Harvest_Tick;
        amountPerTick = GameRules::Wood_Per_Tick;

        // Aðacýn içindeki toplam odun
        setTotalResources(GameRules::Resources_Per_Tree);
    }

    std::string getInfo() override { return "Tree: Odun Kaynagi"; }
};