#pragma once
#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <vector>
#include <set>
#include "Point.h"

/**
 * @brief Static utility class for pathfinding and spatial queries.
 * Implements A* algorithm and heuristic helpers.
 */
class PathFinder {
public:
    /**
     * @brief Calculates the Manhattan distance heuristic between two points.
     */
    static float heuristic(Point a, Point b);

    /**
     * @brief Finds a path from start to goal using the A* algorithm.
     * @param start Starting grid coordinate.
     * @param goal Target grid coordinate.
     * @param mapData Collision map (0 = free, other = obstacle).
     * @param width Map width.
     * @param height Map height.
     * @return A vector of points representing the path.
     */
    static std::vector<Point> findPath(Point start, Point goal, const std::vector<int>& mapData, int width, int height);

    /**
     * @brief Finds the nearest walkable tile using BFS.
     * Useful when a target tile is occupied or reserved.
     */
    static Point findClosestFreeTile(Point target, const std::vector<int>& mapData, int width, int height, const std::set<Point>& reservedTiles);

    /**
     * @brief Determines the best interaction point around a large target (e.g., building).
     * Checks for Line of Sight to ensure the unit walks to an accessible side.
     */
    static Point findBestTargetTile(Point startNode, Point targetCenter, int targetSizeInTiles, const std::vector<int>& mapData, int width, int height, const std::set<Point>& reservedTiles);
};

#endif