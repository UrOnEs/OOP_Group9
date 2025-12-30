#pragma once

#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include "TeamColors.h"
#include "ResourceManager.h"

class Player {
public:
	Player(); // Oyun Ba?lay?nca ortaya ç?kacak

	void selectUnit(sf::RenderWindow& window, const sf::View& camera, bool isShiftHeld); // karakterleri seçme fonksiyonu

	void selectUnitsInRect(const sf::FloatRect& selectionRect, bool isShiftHeld); //

	void renderEntities(sf::RenderWindow& window);

	void removeDeadEntities();

	~Player(); // Yenilgide ortaya ç?kacak

	std::vector<std::shared_ptr<Entity>> entities; // oyuncunun sahip oldu?u bütün karakterler

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

	// Sýrada bekleyen (üretilmeyi bekleyen) asker sayýsý
	int queuedUnits = 0;

	// Sýraya asker eklendiðinde veya sýradan çýktýðýnda bunu çaðýracaðýz
	void addQueuedUnit(int amount) {
		queuedUnits += amount;
	}

	int getCurrentPopulation() {
		return getUnitCount() + queuedUnits;
	}

	ResourceManager playerResources; // --------------------------- bunu private yap getter kullan

	TeamColors getTeamColor() const { return Color; }
	void setTeamColor(TeamColors c) { Color = c; }

	void setName(const std::string& name) { m_name = name; }
	std::string getName() const { return m_name; }

private:
	
	std::string m_name = "Player";

	bool hasBase = true;

	TeamColors Color = TeamColors::Blue; //Di?er oyunculardan ay?rt eden özellik 

	// Bunun ne i? yapt???n? bilmiyorum ama muhtemelen i? yapar :)
	int networkID;

	// limitler burda,her oyuncu için özel belki ev eklersek yükseltmelerde i? yapar
	int unitLimit = 10;
	int buildLimit = 5;




};