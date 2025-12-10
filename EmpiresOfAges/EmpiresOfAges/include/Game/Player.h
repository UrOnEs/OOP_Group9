#pragma once

#include <vector>
#include <memory>
#include "Entity System/Entity.h"
#include "TeamColors.h"
#include "ResourceManager.h"

class Player {
public:
	Player(); // Oyun Baþlayýnca ortaya çýkacak

	std::vector<std::shared_ptr<Entity>> selectUnit(sf::RenderWindow& window); // karakterleri seçme fonksiyonu

	void renderEntities(sf::RenderWindow& window);

	~Player(); // Yenilgide ortaya çýkacak

	std::vector<std::shared_ptr<Entity>> entities; // oyuncunun sahip olduðu bütün karakterler
	
	std::vector<std::shared_ptr<Entity>> selected_entities; // Oyuncunun seçilen karakterleri
	
	std::vector<int> getResources();
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

