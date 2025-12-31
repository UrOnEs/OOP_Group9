#ifndef VILLAGER_H
#define VILLAGER_H

#include "Unit.h"
#include "types.h"
#include "Game/ResourceManager.h"
#include "Game/Player.h"
#include <vector>
#include <memory> 

class ResourceGenerator;
class Building;

enum class VillagerState {
    Idle,
    MovingToResource,
    Harvesting,
    ReturningToBase,
    Garrisoned,
    Building
};

class Villager : public Unit {
private:
    static int IDcounter;
    static int counter;

    VillagerState state = VillagerState::Idle;

    int currentCargo = 0;
    int maxCargo = 10;
    ResourceType cargoType;

    std::weak_ptr<ResourceGenerator> targetResource; // Alýnacak Kaynak
    std::weak_ptr<Building> targetBase; // Kaynaðý býrakacaðý bina
    std::weak_ptr<Building> targetConstruction; // Ýnþa edilecek bina

public:
    Villager();
    ~Villager();

    std::string stats() override;
    int getMaxHealth() const override;

    void startHarvesting(std::shared_ptr<ResourceGenerator> resource);
    void stopHarvesting();
    void updateVillager(float dt, const std::vector<std::shared_ptr<Building>>& buildings, Player& player, const std::vector<int>& mapData, int width, int height);
    void smartMoveTo(sf::Vector2f targetPos, int targetSizeInTiles, const std::vector<int>& mapData, int width, int height);

    void startBuilding(std::shared_ptr<Building> building); // Yeni Emir Fonksiyonu

    // Render fonksiyonunu eziyoruz (Ýçerdeyken çizilmesin diye)
    void render(sf::RenderWindow& window) override;

    std::shared_ptr<Building> findNearestBase(const std::vector<std::shared_ptr<Building>>& buildings);
    void findNearestResource(const std::vector<std::shared_ptr<Building>>& buildings);

    // Çiftlikteyse harvesting sayýlmaz, özel durum
    bool getIsHarvesting() const { return state == VillagerState::Harvesting || state == VillagerState::ReturningToBase; }

    std::shared_ptr<ResourceGenerator> getTargetResource() { return targetResource.lock(); }

    std::string getName() override;

    sf::Texture* getIcon() override {
        if (hasTexture) return (sf::Texture*)sprite.getTexture();
        return nullptr;
    }
};

#endif