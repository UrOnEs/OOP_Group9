#include "ResourceGenerator.h"

class Farm : public ResourceGenerator {
public:
    Farm() {
        buildingType = BuildTypes::Farm;
        interval = 5.0f; // Tarla biraz daha yavaþ olsun
        amountPerTick = 20; // Ama çok yemek versin
    }

    std::string getInfo() override { return "Farm: Yemek Uretir"; }
};
