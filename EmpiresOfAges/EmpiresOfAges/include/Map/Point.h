#pragma once
#ifndef POINT_H
#define POINT_H

#include <tuple> // For std::tie

/**
 * @brief Represents a simple 2D integer coordinate.
 * Used primarily for grid-based map logic and pathfinding.
 */
struct Point {
    int x, y;

    /**
     * @brief Comparison operator for sorting.
     * Required for using Point as a key in std::set and std::map.
     */
    bool operator<(const Point& other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

#endif