#pragma once

#include "Entity System/Entity.h"
#include <vector>

class Unit : public Entity
{
protected:
    float travelSpeed;
    float attackSpeed;

    bool isMoving;

public:
    // Hýz deðerine sistemlerin ulaþmasý lazým
    float getSpeed() const { return travelSpeed; }

    std::vector<sf::Vector2f> path;

    void setGridPosition(int tx, int ty) {
        // 32, senin TileSize deðerin. Bunu GameRules'dan çekmek daha iyi olur ama þimdilik 32 yazalým.
        float pixelX = tx * 32.0f;
        float pixelY = ty * 32.0f;

        // Tam karenin ortasýnda dursun istiyorsan +16 eklersin
        setPosition(sf::Vector2f(pixelX, pixelY));
    }
};