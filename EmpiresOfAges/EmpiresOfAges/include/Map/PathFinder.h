// PathFinder.h

#ifndef PATHFINDER_H
#define PATHFINDER_H


#pragma once
#include <vector>
#include <set>
#include "Point.h"

class PathFinder {
public:
    // Sadece statik fonksiyonlar içerdiði için constructor'a gerek yok

    // Heuristic hesaplama (Manhattan)
    static float heuristic(Point a, Point b);

    // A* Algoritmasý ile yol bulma
    static std::vector<Point> findPath(Point start, Point goal, const std::vector<int>& mapData, int width, int height);

    // BFS Algoritmasý ile en yakýn boþ kareyi bulma (Formasyon için)
    static Point findClosestFreeTile(Point target, const std::vector<int>& mapData, int width, int height, const std::set<Point>& reservedTiles);
};
#endif