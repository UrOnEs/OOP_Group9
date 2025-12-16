#include "Map/PathFinder.h"
#include <queue>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits> 

// Yardýmcý: Ýki nokta arasýnda duvar var mý? (Raycast Mantýðý)
// Bu fonksiyon "Pixel Pixel" hareketin anahtarýdýr.
bool isLineClear(Point start, Point end, const std::vector<int>& mapData, int width, int height) {
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        // Duvar kontrolü
        if (mapData[x0 + y0 * width] != 0) return false;

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    return true;
}

// Yolu pürüzsüzleþtirme (String Pulling)
// Zikzaklarý silip pixel pixel düz yol yapar
std::vector<Point> smoothPath(const std::vector<Point>& path, const std::vector<int>& mapData, int width, int height) {
    if (path.size() < 3) return path;

    std::vector<Point> smoothed;
    smoothed.push_back(path[0]);

    int currentIdx = 0;
    while (currentIdx < path.size() - 1) {
        int checkIdx = path.size() - 1; // En sondan geriye doðru bak

        while (checkIdx > currentIdx + 1) {
            // Eðer þu anki noktadan, ilerideki noktaya DÜMDÜZ çizgi çekebiliyorsam
            // Aradaki zikzaklarý atla!
            if (isLineClear(path[currentIdx], path[checkIdx], mapData, width, height)) {
                break; // En uzak temiz noktayý bulduk
            }
            checkIdx--;
        }

        smoothed.push_back(path[checkIdx]);
        currentIdx = checkIdx;
    }

    return smoothed;
}

float PathFinder::heuristic(Point a, Point b) {
    // Euclidean (Kuþ bakýþý) mesafe daha doðal sonuç verir
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

std::vector<Point> PathFinder::findPath(Point start, Point goal, const std::vector<int>& mapData, int width, int height) {
    using Element = std::pair<float, Point>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> frontier;

    frontier.push({ 0.0f, start });
    std::map<Point, Point> cameFrom;
    std::map<Point, float> costSoFar;

    cameFrom[start] = start;
    costSoFar[start] = 0.0f;

    int iterations = 0;
    const int MAX_ITERATIONS = 5000;
    Point closestReach = start;
    float minH = std::numeric_limits<float>::max();
    bool targetFound = false;

    while (!frontier.empty()) {
        Point current = frontier.top().second;
        frontier.pop();
        iterations++;

        if (current == goal) { targetFound = true; break; }
        if (iterations > MAX_ITERATIONS) break;

        float h = heuristic(current, goal);
        if (h < minH) { minH = h; closestReach = current; }

        // --- YÖNLER ---
        // 0-3: Düz (Sað, Sol, Aþaðý, Yukarý)
        // 4-7: Çapraz (Sað-Alt, Sað-Üst, Sol-Alt, Sol-Üst)
        int dx[8] = { 1, -1, 0, 0,   1,  1, -1, -1 };
        int dy[8] = { 0, 0, 1, -1,   1, -1,  1, -1 };
        float costs[8] = { 1.0f, 1.0f, 1.0f, 1.0f,  1.414f, 1.414f, 1.414f, 1.414f };

        for (int i = 0; i < 8; ++i) {
            Point next = { current.x + dx[i], current.y + dy[i] };

            // 1. Sýnýr Kontrolü
            if (next.x < 0 || next.y < 0 || next.x >= width || next.y >= height) continue;

            // 2. Duvar Kontrolü
            if (mapData[next.x + next.y * width] != 0) continue;

            // --- 3. KÖÞE KESME (CORNER CUTTING) ENGELLEME ---
            // Burasý "Duvarýn içine girme" sorununu çözer!
            // Çapraz gidiyorsak (i >= 4), yanýndaki iki düz karenin de boþ olmasý lazým.
            if (i >= 4) {
                // Örneðin: Sað-Aþaðý gidiyorsak (dx=1, dy=1), hem Sað'ýn hem Aþaðý'nýn boþ olmasý lazým.
                bool horizontalWall = (mapData[(current.x + dx[i]) + current.y * width] != 0);
                bool verticalWall = (mapData[current.x + (current.y + dy[i]) * width] != 0);

                // Eðer yanlardan biri bile doluysa, çapraz geçiþi yasakla
                if (horizontalWall || verticalWall) continue;
            }

            float newCost = costSoFar[current] + costs[i];
            if (costSoFar.find(next) == costSoFar.end() || newCost < costSoFar[next]) {
                costSoFar[next] = newCost;
                float priority = newCost + heuristic(next, goal);
                frontier.push({ priority, next });
                cameFrom[next] = current;
            }
        }
    }

    // --- YOL OLUÞTURMA ---
    std::vector<Point> path;
    Point reconstructionStart = targetFound ? goal : closestReach;

    if (cameFrom.find(reconstructionStart) == cameFrom.end()) return path;

    Point current = reconstructionStart;
    while (current != start) {
        path.push_back(current);
        current = cameFrom[current];
    }
    std::reverse(path.begin(), path.end());

    // --- SON DOKUNUÞ: YOLU YUMUÞAT (Pixel Pixel Hissi) ---
    // Eðer bunu istemezsen aþaðýdaki satýrý silip sadece "return path;" diyebilirsin.
    return smoothPath(path, mapData, width, height);
}

// ... findClosestFreeTile fonksiyonu aynen kalabilir ...
Point PathFinder::findClosestFreeTile(Point target, const std::vector<int>& mapData, int width, int height, const std::set<Point>& reservedTiles) {
    std::queue<Point> q;
    std::set<Point> visited;
    q.push(target);
    visited.insert(target);
    int limit = 0;
    while (!q.empty()) {
        Point curr = q.front(); q.pop();
        if (++limit > 500) return target;

        bool isWall = (curr.x >= 0 && curr.y >= 0 && curr.x < width && curr.y < height) ? (mapData[curr.x + curr.y * width] != 0) : true;
        bool isReserved = (reservedTiles.find(curr) != reservedTiles.end());

        if (!isWall && !isReserved) return curr;

        Point neighbors[4] = { {curr.x + 1, curr.y}, {curr.x - 1, curr.y}, {curr.x, curr.y + 1}, {curr.x, curr.y - 1} };
        for (Point next : neighbors) {
            if (next.x >= 0 && next.y >= 0 && next.x < width && next.y < height) {
                if (visited.find(next) == visited.end()) { visited.insert(next); q.push(next); }
            }
        }
    }
    return target;
}

