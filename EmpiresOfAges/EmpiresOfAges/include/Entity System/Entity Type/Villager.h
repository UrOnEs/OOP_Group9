#ifndef VILLAGER_H
#define VILLAGER_H

#include "Unit.h"
#include "types.h"

class Villager : public Unit {
private:
	static int IDcounter;
	static int counter; // belli bir sayýyý geçmediðinden emin olmalýyýz
	
	bool isBusy = false;
public:
	std::string stats() override;

	Villager();//Köylü oluþturcaz - Veriler GameRules'da

	~Villager(); // zaten biliyon

};

#endif // !VILLAGER_H
