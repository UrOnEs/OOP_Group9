#ifndef BUILDING_H
#define BUILDING_H

#include "Entity System/Entity.h"
#include "types.h"
#include "Soldier.h"

class Building : public Entity {
private:
	static int IDcounter;
	static int counter; // belli bir sayýyý geçmediðinden emin olmalýyýz
public:
	BuildTypes buildingType;


	Building(); //Binayý dikiyoz - Veriler GameRules'da

	~Building(); // zaten biliyon

	bool produce();
	bool produce(SoldierTypes& Sproduct);
};

#endif // !BUILDING_H
