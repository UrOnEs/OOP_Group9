#pragma once
#ifndef TYPES_H
#define TYPES_H

/**
 * @brief Enumeration of all building and resource types in the game.
 */
enum class BuildTypes {
    Barrack,
    Farm,
    WoodPlace,
    StoneMine,
    GoldMine,
    House,
    Tree,
    TownCenter,
    Stone,
    Gold,
    Mountain
};

/**
 * @brief Enumeration of combat unit types.
 */
enum class SoldierTypes {
    Barbarian,
    Archer,
    Wizard
};

#endif // !TYPES_H