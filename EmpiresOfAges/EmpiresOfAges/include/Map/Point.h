// Point.h
#ifndef POINT_H
#define POINT_H


#pragma once
#include <tuple> // std::tie için

struct Point {
    int x, y;

    // std::set ve std::map içinde anahtar olarak kullanýlabilmesi için gerekli operatörler
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