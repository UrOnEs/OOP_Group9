#include "Entity System/Entity Type/Unit.h"
#include "Game/GameRules.h" // GameRules'u eklemeyi unutma
#include <cmath>

int Unit::entityCounter = 0;

// Varsayýlan Yapýcý
Unit::Unit()
    : m_tileSize(GameRules::TileSize), // Varsayýlan olarak GameRules'daki deðeri al
    m_isMoving(false),
    travelSpeed(100.f)
{
    // --- DEÐÝÞÝKLÝK: Yarýçap GameRules'dan geliyor ---
    shape.setRadius(GameRules::UnitRadius);
    shape.setOrigin(GameRules::UnitRadius, GameRules::UnitRadius);
}



// Parametreli Yapýcý
Unit::Unit(int startGridX, int startGridY, int tileSize)
    : m_tileSize(tileSize), m_isMoving(false), travelSpeed(100.f)
{
    // --- DEÐÝÞÝKLÝK: Yarýçap GameRules'dan geliyor ---
    // tileSize ne gelirse gelsin, askerin boyutu sabittir.
    float r = GameRules::UnitRadius;

    shape.setRadius(r);
    shape.setOrigin(r, r);
    shape.setFillColor(sf::Color::Red);

    // Konumlandýrma: Askeri yine karenin (tileSize) ortasýna koyuyoruz
    sf::Vector2f startPixel(startGridX * tileSize + tileSize / 2.0f, startGridY * tileSize + tileSize / 2.0f);
    shape.setPosition(startPixel);
    m_targetPos = startPixel;
}

void Unit::moveTo(sf::Vector2f targetWorldPos) {
    m_targetPos = targetWorldPos;
    m_isMoving = true;

    // HAREKETÝN BAÞLAMASI ÝÇÝN YOL LÝSTESÝNÝ DOLDURMALIYIZ
    m_path.clear();
    m_path.push_back(targetWorldPos);
}

// ------------------- FÝZÝKSEL HAREKET ---------------------
void Unit::update(float dt, const std::vector<int>& mapData, int width, int height) {
    // Hareket etmiyor veya yol bittiyse dur
    if (!m_isMoving || m_path.empty()) return;

    sf::Vector2f currentPos = shape.getPosition();
    sf::Vector2f direction = m_targetPos - currentPos;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 4.0f) {
        shape.setPosition(m_targetPos);
        m_path.erase(m_path.begin());

        if (m_path.empty()) {
            m_isMoving = false;
        }
        else {
            m_targetPos = m_path[0];
        }
        return;
    }

    sf::Vector2f normalizedDir = direction / distance;
    sf::Vector2f velocity = normalizedDir * travelSpeed * dt;

    sf::Vector2f nextPosX = currentPos;
    nextPosX.x += velocity.x;

    if (checkCollision(nextPosX, mapData, width, height)) {
        if (velocity.y != 0) velocity.y = (velocity.y > 0 ? 1.f : -1.f) * travelSpeed * dt;
    }
    else {
        currentPos.x = nextPosX.x;
    }

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
        // Burada m_tileSize (64) kullanýldýðý için grid hesabý doðru çalýþýr
        int tx = static_cast<int>(p.x) / m_tileSize;
        int ty = static_cast<int>(p.y) / m_tileSize;

        if (tx < 0 || ty < 0 || tx >= width || ty >= height) return true;
        if (mapData[tx + ty * width] != 0) return true;
    }
    return false;
}

void Unit::render(sf::RenderWindow& window) {
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

void Unit::setPath(const std::vector<sf::Vector2f>& pathPoints) {
    m_path = pathPoints;
    if (!m_path.empty()) {
        m_isMoving = true;
        m_targetPos = m_path[0];
    }
    else {
        m_isMoving = false;
    }
}