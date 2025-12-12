#include "ResourceGenerator.h"

class StoneMine : public ResourceGenerator {
public:
    StoneMine() {
        buildingType = BuildTypes::StoneMines; // Enum'da ismini düzeltmen gerekebilir
        interval = 2.0f; // Hýzlý kazsýn
        amountPerTick = 5; // Az taþ versin
    }

    std::string getInfo() override { return "Mine: Tas Uretir"; }
};
