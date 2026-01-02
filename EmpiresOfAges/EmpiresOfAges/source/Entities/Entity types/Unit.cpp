#include "Entity System/Entity Type/Unit.h"
#include "Game/GameRules.h" 
#include <cmath>

int Unit::entityCounter = 0;

Unit::Unit()
    : m_tileSize(GameRules::TileSize),
    m_isMoving(false),
    travelSpeed(100.f)
{
    shape.setRadius(GameRules::UnitRadius);
    shape.setOrigin(GameRules::UnitRadius, GameRules::UnitRadius);
}

Unit::Unit(int startGridX, int startGridY, int tileSize)
    : m_tileSize(tileSize), m_isMoving(false), travelSpeed(100.f)
{
    float r = GameRules::UnitRadius;
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
    m_path.clear();
    m_path.push_back(targetWorldPos);
}

void Unit::update(float dt, const std::vector<int>& mapData, int width, int height) {
    if (!m_isMoving || m_path.empty()) return;

    sf::Vector2f currentPos = shape.getPosition();
    sf::Vector2f direction = m_targetPos - currentPos;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 4.0f) {
        this->setPosition(m_targetPos);
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

    // X axis movement
    sf::Vector2f nextPosX = currentPos;
    nextPosX.x += velocity.x;

    if (checkCollision(nextPosX, mapData, width, height)) {
        // Collision detected on X
    }
    else {
        currentPos.x = nextPosX.x;
    }

    // Y axis movement
    sf::Vector2f nextPosY = currentPos;
    nextPosY.y += velocity.y;

    if (!checkCollision(nextPosY, mapData, width, height)) {
        currentPos.y = nextPosY.y;
    }

    this->setPosition(currentPos);
}

bool Unit::checkCollision(const sf::Vector2f& newPos, const std::vector<int>& mapData, int width, int height) {
    // Collision tolerance (reduce radius by 25% for smoother navigation)
    float r = shape.getRadius() * 0.75f;

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
    Entity::render(window);

    // Render Attack Range Indicator
    if (isSelected && range > 25.0f) {
        sf::CircleShape rangeCircle(range);
        rangeCircle.setOrigin(range, range);
        rangeCircle.setPosition(getPosition());

        rangeCircle.setFillColor(sf::Color::Transparent);
        rangeCircle.setOutlineColor(sf::Color(255, 255, 255, 100));
        rangeCircle.setOutlineThickness(1.0f);
        rangeCircle.setPointCount(50);
        rangeCircle.setScale(1.0f, 0.6f); // Isometric perspective

        window.draw(rangeCircle);
    }
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