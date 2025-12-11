#include "Map/PathFinder.h"
#include <queue>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits> // float max için

float PathFinder::heuristic(Point a, Point b) {
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

std::vector<Point> PathFinder::findPath(Point start, Point goal, const std::vector<int>& mapData, int width, int height) {
    using Element = std::pair<float, Point>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> frontier;

    frontier.push({ 0.0f, start });
    std::map<Point, Point> cameFrom;
    std::map<Point, float> costSoFar;

    cameFrom[start] = start;
    costSoFar[start] = 0.0f;

    // --- YENÝ EKLENTÝ: SABIR SINIRI VE EN YAKIN NOKTA ---
    int iterations = 0;
    const int MAX_ITERATIONS = 3000; // Algoritma en fazla 3000 kareye baksýn (Performans ayarý)

    Point closestReach = start;      // Hedefe en çok yaklaþabildiðimiz nokta
    float minH = std::numeric_limits<float>::max(); // En küçük mesafe skoru

    bool targetFound = false;

    while (!frontier.empty()) {
        Point current = frontier.top().second;
        frontier.pop();

        // Döngü sayacýný artýr
        iterations++;

        // Hedefe ulaþtýk mý?
        if (current == goal) {
            targetFound = true;
            break;
        }

        // --- GÜVENLÝK KÝLÝDÝ ---
        // Eðer çok fazla aradýysak ve bulamadýysak pes et.
        if (iterations > MAX_ITERATIONS) {
            // std::cout << "Yol bulunamadi (Cok uzak veya kapali), en yakin noktaya gidiliyor.\n";
            break;
        }

        // Hedefe ne kadar yakýnýz? (En iyi alternatifi kaydet)
        float h = heuristic(current, goal);
        if (h < minH) {
            minH = h;
            closestReach = current;
        }

        Point neighbors[4] = { {current.x + 1, current.y}, {current.x - 1, current.y}, {current.x, current.y + 1}, {current.x, current.y - 1} };

        for (Point next : neighbors) {
            if (next.x < 0 || next.y < 0 || next.x >= width || next.y >= height) continue;
            if (mapData[next.x + next.y * width] != 0) continue; // Duvar

            float newCost = costSoFar[current] + 1.0f;
            if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                costSoFar[next] = newCost;
                float priority = newCost + heuristic(next, goal);
                frontier.push({ priority, next });
                cameFrom[next] = current;
            }
        }
    }

    // Yol oluþturma kýsmý
    std::vector<Point> path;

    // Eðer hedef bulunduysa 'goal' noktasýndan geri gel, 
    // Bulunamadýysa (duvar arkasýysa) 'closestReach' noktasýndan geri gel.
    Point reconstructionStart = targetFound ? goal : closestReach;

    // Eðer hiç yol yoksa (baþlangýç noktasýndan kýmýldayamadýysak)
    if (cameFrom.find(reconstructionStart) == cameFrom.end()) {
        return path;
    }

    Point current = reconstructionStart;
    while (current != start) {
        path.push_back(current);
        current = cameFrom[current];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

Point PathFinder::findClosestFreeTile(Point target, const std::vector<int>& mapData, int width, int height, const std::set<Point>& reservedTiles) {
    // BFS algoritmasý yakýndaki alaný taradýðý için genelde hýzlýdýr, 
    // ancak o da kapalý alanda kalýrsa sonsuz döngüye girebilir.
    // Ona da bir limit koyalým.

    std::queue<Point> q;
    std::set<Point> visited;
    q.push(target);
    visited.insert(target);

    int limit = 0;
    const int BFS_LIMIT = 500; // En yakýn boþ kareyi bulmak için max 500 kareye bak

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        limit++;
        if (limit > BFS_LIMIT) return target; // Bulamazsan zorlama, olduðu yeri döndür.

        bool isWall = (mapData[curr.x + curr.y * width] != 0);
        bool isReserved = (reservedTiles.find(curr) != reservedTiles.end());

        if (!isWall && !isReserved) {
            return curr;
        }

        Point neighbors[4] = { {curr.x + 1, curr.y}, {curr.x - 1, curr.y}, {curr.x, curr.y + 1}, {curr.x, curr.y - 1} };
        for (Point next : neighbors) {
            if (next.x >= 0 && next.y >= 0 && next.x < width && next.y < height) {
                if (visited.find(next) == visited.end()) {
                    visited.insert(next);
                    q.push(next);
                }
            }
        }
    }
    return target;
}