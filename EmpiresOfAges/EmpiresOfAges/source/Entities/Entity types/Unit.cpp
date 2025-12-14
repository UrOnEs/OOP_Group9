#include "Entity System/Entity Type/Unit.h"
#include <cmath>

// --- DÜZELTME: Varsayýlan Constructor ---
Unit::Unit() : m_tileSize(32), m_isMoving(false), travelSpeed(100.f) {
    // Varsayýlan deðerler (Hata almamak için)
    shape.setRadius(12.f);
    shape.setOrigin(12.f, 12.f);
}

Unit::Unit(int startGridX, int startGridY, int tileSize)
    : m_tileSize(tileSize), m_isMoving(false), travelSpeed(100.f)
{
    float r = tileSize / 2.0f - 4.0f;
    shape.setRadius(r);
    shape.setOrigin(r, r);
    shape.setFillColor(sf::Color::Red);

    sf::Vector2f startPixel(startGridX * tileSize + tileSize / 2.0f, startGridY * tileSize + tileSize / 2.0f);
    shape.setPosition(startPixel);
    m_targetPos = startPixel;
}

void Unit::moveTo(sf::Vector2f targetWorldPos) {
    m_targetPos = targetWorldPos;
    m_isMoving = true;
}

// --- FÝZÝKSEL HAREKET VE KAYMA (WALL SLIDING) ---
void Unit::update(float dt, const std::vector<int>& mapData, int width, int height) {
    if (!m_isMoving) return;

    sf::Vector2f currentPos = shape.getPosition();
    sf::Vector2f direction = m_targetPos - currentPos;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 2.0f) {
        shape.setPosition(m_targetPos);
        m_isMoving = false;
        return;
    }

    // Yönü normalize et
    sf::Vector2f normalizedDir = direction / distance;

    // Hýzý hesapla
    sf::Vector2f velocity = normalizedDir * travelSpeed * dt;

    // --- X EKSENÝ KONTROLÜ ---
    sf::Vector2f nextPosX = currentPos;
    nextPosX.x += velocity.x;

    if (checkCollision(nextPosX, mapData, width, height)) {
        // X'te duvara çarptýk! 
        // X'i iptal et ama Y'yi hýzlandýr (Duvar boyunca tam gaz kaymasý için)
        // Eðer Y yönünde hareket varsa, Y hýzýný 'travelSpeed' yap.
        if (velocity.y != 0) {
            velocity.y = (velocity.y > 0 ? 1.f : -1.f) * travelSpeed * dt;
        }
    }
    else {
        // Çarpma yoksa X hareketini onayla
        currentPos.x = nextPosX.x;
    }

    // --- Y EKSENÝ KONTROLÜ ---
    sf::Vector2f nextPosY = currentPos;
    nextPosY.y += velocity.y; // Güncellenmiþ velocity.y'yi kullanýyoruz

    if (checkCollision(nextPosY, mapData, width, height)) {
        // Y'de duvara çarptýk!
        // Y'yi iptal et ama X'i hýzlandýr (Eðer X hareketi zaten onaylandýysa, tekrar artýrmýyoruz, sadece Y'yi kesiyoruz)
        // Ancak bu adýmda X çoktan oynatýldýðý için geriye dönük hýzlandýrma yapmýyoruz, sadece Y'yi durduruyoruz.
    }
    else {
        // Çarpma yoksa Y hareketini onayla
        currentPos.y = nextPosY.y;
    }

    shape.setPosition(currentPos);
}

bool Unit::checkCollision(const sf::Vector2f& newPos, const std::vector<int>& mapData, int width, int height) {
    float r = shape.getRadius();
    sf::FloatRect bounds(newPos.x - r, newPos.y - r, r * 2, r * 2);

    sf::Vector2f corners[4] = {
        {bounds.left, bounds.top},
        {bounds.left + bounds.width, bounds.top},
        {bounds.left, bounds.top + bounds.height},
        {bounds.left + bounds.width, bounds.top + bounds.height}
    };

    for (const auto& p : corners) {
        int tx = static_cast<int>(p.x) / m_tileSize;
        int ty = static_cast<int>(p.y) / m_tileSize;

        if (tx < 0 || ty < 0 || tx >= width || ty >= height) return true;
        if (mapData[tx + ty * width] != 0) return true;
    }
    return false;
}

void Unit::render(sf::RenderWindow& window) {
    // Entity'den gelen seçim kontrolü
    if (isSelected) {
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::Green);
    }
    else {
        shape.setOutlineThickness(0);
    }
    window.draw(shape);
}

Point Unit::getGridPoint() const {
    return { static_cast<int>(shape.getPosition().x / m_tileSize),
             static_cast<int>(shape.getPosition().y / m_tileSize) };
}