#pragma once

#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include "TeamColors.h"
#include "ResourceManager.h"

class Player {
public:
	Player(); // Oyun Baþlayýnca ortaya çýkacak

	std::vector<std::shared_ptr<Entity>> selectUnit(sf::RenderWindow& window, const sf::View& camera); // karakterleri seçme fonksiyonu

	void renderEntities(sf::RenderWindow& window);

	~Player(); // Yenilgide ortaya çýkacak

	std::vector<std::shared_ptr<Entity>> entities; // oyuncunun sahip olduðu bütün karakterler
	
	std::vector<std::shared_ptr<Entity>> selected_entities; // Oyuncunun seçilen karakterleri
		
	std::vector<int> getResources();

	std::vector<std::shared_ptr<Entity>> getEntities();

	void addWood(int amount) { playerResources.add(ResourceType::Wood, amount); }
	void addGold(int amount) { playerResources.add(ResourceType::Gold, amount); }
	void addFood(int amount) { playerResources.add(ResourceType::Food, amount); }
	void addStone(int amount) { playerResources.add(ResourceType::Stone, amount); }

	void addEntity(std::shared_ptr<Entity> entity) { // Asker Eklemek için
		entities.push_back(entity);
	}

	bool addUnitLimit(int);

	int getUnitLimit();

	int getUnitCount();

	bool setUnitLimit(int);

private:
	ResourceManager playerResources;
	
	bool hasBase = true;

	TeamColors Color; //Diðer oyunculardan ayýrt eden özellik 
	
	// Bunun ne iþ yaptýðýný bilmiyorum ama muhtemelen iþ yapar :)
	int networkID; 

	// limitler burda,her oyuncu için özel belki ev eklersek yükseltmelerde iþ yapar
	int unitLimit = 10;	
	int buildLimit = 5; 




};

