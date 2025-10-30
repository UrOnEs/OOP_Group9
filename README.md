# Group 9

## 👥 Group Members
- Cahit Onur Enoğlu
- Eren Köse
- Hacı Salih Toker
- Yusuf Yücel

## 📝 Project Overview
Our project is a **real-time strategy (RTS) game** developed in C++ using the SFML library. It features a tile-based map system, unit control, resource gathering and building construction.

## 🏗️ Main Class Structure

### Map / Tile
The `Map` class manages the tile-based world, composed of `Tile` objects holding terrain type (grass, stone, water, forest), passability, and coordinates.

### Entity (abstract)
Base class for all game entities, containing shared attributes like position, health, and selection state.

### Unit
Derived from `Entity`; handles movement, attacking, and resource gathering.

### Building
Derived from `Entity`; responsible for unit production and upgrades.

### UI
Manages all interface components. Includes subclasses such as `Button`, `Panel`, and `ResourceDisplay`.

`Button` uses polymorphism for custom click behavior depending on the button type.

### PathFinder / Node
Implements the **A* algorithm** to compute optimal paths using data from the `Map`. `Node` represents individual points in the search process.

## 🎯 Design Philosophy
This structure emphasizes modular, object-oriented design, ensuring scalability and maintainability as game complexity increases.
