#pragma once
#include "Entity System/Entity.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Map/Point.h"

class Unit : public Entity {
public:
    // Soldier sýnýfý "Soldier()" dediðinde arka planda "Unit()" çaðrýlýr.
    // Bu olmazsa "uygun varsayýlan oluþturucu yok" hatasý alýrsýn.
    Unit();
    Unit(int startGridX, int startGridY, int tileSize);

    // Update fonksiyonu (Harita verisiyle)
    void update(float dt, const std::vector<int>& mapData, int mapWidth, int mapHeight);

    void render(sf::RenderWindow& window) override;

    void setPath(const std::vector<sf::Vector2f>& pathPoints);// Askerin yeni rotasýný belirleyen fonksiyon

    // --- HAREKET SÝSTEMÝ ---
    void moveTo(sf::Vector2f targetWorldPos);
    Point getGridPoint() const;
    static int getPopulation() { return entityCounter; };

    // Getterlar (Eski kodlarýn çalýþmasý için)
    float getSpeed() const { return travelSpeed; }

protected:
    float travelSpeed;
    static int entityCounter;

    // Hareket Deðiþkenleri
    bool m_isMoving;
    sf::Vector2f m_targetPos;
    int m_tileSize;

    std::vector<sf::Vector2f> m_path; // Rota listesi

private:
    // Çarpýþma Kontrolü
    bool checkCollision(const sf::Vector2f& newPos, const std::vector<int>& mapData, int width, int height);
};