#include "Map/PathFinder.h"
#include <queue>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits> 
#include <set> // Set kütüphanesi eklendi

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
        // 1. Temel Duvar Kontrolü
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            if (mapData[x0 + y0 * width] != 0) return false;
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        bool stepX = false;
        bool stepY = false;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
            stepX = true;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
            stepY = true;
        }

        // --- KÖÞE KESME KONTROLÜ ---
        if (stepX && stepY) {
            int checkX = x0 - sx;
            int checkY = y0 - sy;

            bool wall1 = false;
            if (x0 >= 0 && x0 < width && checkY >= 0 && checkY < height)
                wall1 = (mapData[x0 + checkY * width] != 0);

            bool wall2 = false;
            if (checkX >= 0 && checkX < width && y0 >= 0 && y0 < height)
                wall2 = (mapData[checkX + y0 * width] != 0);

            if (wall1 || wall2) return false;
        }
    }
    return true;
}

// Yolu pürüzsüzleþtirme (String Pulling)
std::vector<Point> smoothPath(const std::vector<Point>& path, const std::vector<int>& mapData, int width, int height) {
    if (path.size() < 3) return path;

    std::vector<Point> smoothed;
    smoothed.push_back(path[0]);

    int currentIdx = 0;
    while (currentIdx < path.size() - 1) {
        int checkIdx = path.size() - 1;

        while (checkIdx > currentIdx + 1) {
            if (isLineClear(path[currentIdx], path[checkIdx], mapData, width, height)) {
                break;
            }
            checkIdx--;
        }

        smoothed.push_back(path[checkIdx]);
        currentIdx = checkIdx;
    }

    return smoothed;
}

float PathFinder::heuristic(Point a, Point b) {
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

// --- GÜNCELLENMÝÞ HEDEF SEÇÝMÝ FONKSÝYONU ---
Point PathFinder::findBestTargetTile(Point startNode, Point targetCenter, int targetSizeInTiles, const std::vector<int>& mapData, int width, int height, const std::set<Point>& reservedTiles) {

    // 1. Binanýn etrafýndaki alaný belirle
    int halfSize = targetSizeInTiles / 2;
    // Çift sayýlarda (örn 4x4) merkezden hafif kaydýrma gerekebilir ama genelde sol üst köþeyi baz alýrýz.
    int rectX = targetCenter.x - halfSize;
    int rectY = targetCenter.y - halfSize;
    int rectW = targetSizeInTiles;
    int rectH = targetSizeInTiles;

    std::vector<Point> candidates;

    // 2. Çevredeki tüm BOÞ kareleri topla
    // (Binanýn 1 kare dýþýndaki çemberi tarýyoruz)
    for (int x = rectX - 1; x <= rectX + rectW; ++x) {
        for (int y = rectY - 1; y <= rectY + rectH; ++y) {

            // Binanýn kendisini (içini) atla
            if (x >= rectX && x < rectX + rectW && y >= rectY && y < rectY + rectH) continue;

            if (x < 0 || y < 0 || x >= width || y >= height) continue;

            Point current = { x, y };

            // Duvar kontrolü
            if (mapData[x + y * width] != 0) continue;

            // Rezerve kontrolü
            if (reservedTiles.find(current) != reservedTiles.end()) continue;

            candidates.push_back(current);
        }
    }

    // Hiç boþ yer yoksa (tamamen sarýlýysa) klasik yönteme dön
    if (candidates.empty()) {
        return findClosestFreeTile(targetCenter, mapData, width, height, reservedTiles);
    }

    // 3. Adaylarý köylüye olan mesafeye göre sýrala (En yakýndan en uzaða)
    std::sort(candidates.begin(), candidates.end(), [startNode](Point a, Point b) {
        return heuristic(startNode, a) < heuristic(startNode, b);
        });

    // --- KRÝTÝK DEÐÝÞÝKLÝK: GÖRÜÞ AÇISI KONTROLÜ ---
    // Sadece en yakýný seçmek yerine, önü açýk olan (isLineClear) ilk adayý seçiyoruz.
    // Bu sayede köylü aðacýn arkasýndaki kareye gitmeye çalýþmak yerine, 
    // biraz yan taraftaki ama dümdüz yürüyebileceði kareye gider.

    int checkLimit = 0;
    for (const auto& p : candidates) {
        if (isLineClear(startNode, p, mapData, width, height)) {
            return p; // Bulduk! Önü açýk ve yakýn.
        }

        // Çok fazla iþlem yapmamak için en yakýn 10 kareye bakmak yeterli.
        checkLimit++;
        if (checkLimit > 10) break;
    }

    // Eðer hiçbirinin önü açýk deðilse (örn. duvarýn arkasýndaysak), mecburen en yakýn olaný seç.
    // A* algoritmasý yolu çizecektir.
    return candidates[0];
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

        int dx[8] = { 1, -1, 0, 0,   1,  1, -1, -1 };
        int dy[8] = { 0, 0, 1, -1,   1, -1,  1, -1 };
        float costs[8] = { 1.0f, 1.0f, 1.0f, 1.0f,  1.414f, 1.414f, 1.414f, 1.414f };

        for (int i = 0; i < 8; ++i) {
            Point next = { current.x + dx[i], current.y + dy[i] };

            if (next.x < 0 || next.y < 0 || next.x >= width || next.y >= height) continue;

            if (mapData[next.x + next.y * width] != 0) continue;

            // Çapraz hareket kontrolü (Köþe sýkýþmasýný engelleme)
            if (i >= 4) {
                bool horizontalWall = (mapData[(current.x + dx[i]) + current.y * width] != 0);
                bool verticalWall = (mapData[current.x + (current.y + dy[i]) * width] != 0);
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

    std::vector<Point> path;
    Point reconstructionStart = targetFound ? goal : closestReach;

    if (cameFrom.find(reconstructionStart) == cameFrom.end()) return path;

    Point current = reconstructionStart;
    while (current != start) {
        path.push_back(current);
        current = cameFrom[current];
    }
    std::reverse(path.begin(), path.end());

    return smoothPath(path, mapData, width, height);
}

// Eski fonksiyon (Yedek olarak kalabilir veya silinebilir)
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