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

// ------------------- FÝZÝKSEL HAREKET ---------------------
void Unit::update(float dt, const std::vector<int>& mapData, int width, int height) {
    // Hareket etmiyor veya yol bittiyse dur
    if (!m_isMoving || m_path.empty()) return;

    sf::Vector2f currentPos = shape.getPosition();
    sf::Vector2f direction = m_targetPos - currentPos;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Hedefe çok yaklaþtýk mý? (Tolerans 4 piksel)
    if (distance < 4.0f) {
        shape.setPosition(m_targetPos); // Tam noktaya otur

        // Bu duraðý listeden sil (Çünkü vardýk)
        m_path.erase(m_path.begin());

        // Baþka durak kaldý mý?
        if (m_path.empty()) {
            m_isMoving = false; // Yol bitti
        }
        else {
            m_targetPos = m_path[0]; // Sýradaki duraðý hedefle
        }
        return;
    }

    // --- HAREKET MANTIÐI ---
    sf::Vector2f normalizedDir = direction / distance;
    sf::Vector2f velocity = normalizedDir * travelSpeed * dt;

    // X Ekseninde Dene
    sf::Vector2f nextPosX = currentPos;
    nextPosX.x += velocity.x;

    // Basit çarpýþma kontrolü (Güvenlik için kalsýn)
    if (checkCollision(nextPosX, mapData, width, height)) {
        // Eðer duvara sürterse kaymaya devam etsin
        if (velocity.y != 0) velocity.y = (velocity.y > 0 ? 1.f : -1.f) * travelSpeed * dt;
    }
    else {
        currentPos.x = nextPosX.x;
    }

    // Y Ekseninde Dene
    sf::Vector2f nextPosY = currentPos;
    nextPosY.y += velocity.y;

    if (!checkCollision(nextPosY, mapData, width, height)) {
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

//-------------Rota Atama Fonksiyonu-----------------------------
void Unit::setPath(const std::vector<sf::Vector2f>& pathPoints) {
    m_path = pathPoints; // Listeyi kaydet

    // Eðer liste doluysa ilk duraða doðru yola çýk
    if (!m_path.empty()) {
        m_isMoving = true;
        m_targetPos = m_path[0];
    }
    else {
        m_isMoving = false;
    }
}