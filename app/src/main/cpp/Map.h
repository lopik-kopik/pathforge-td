#ifndef TOWERDEFENSE_MAP_H
#define TOWERDEFENSE_MAP_H

#include <vector>
#include <utility>

enum class CellType {
    GRASS,
    PATH,
    BLOCKED,
    TOWER
};

enum class MapType {
    FOREST,     // Default - green, trees
    DESERT,     // Sand, cacti, sandstorms
    SNOW,       // Ice, snowmen, slippery
    DUNGEON     // Dark, rocks, lava
};

struct Waypoint {
    float x, y;
};

class Map {
public:
    static constexpr int COLS = 10;
    static constexpr int ROWS = 16;

    Map();
    explicit Map(MapType type);

    CellType getCell(int col, int row) const;
    void setCell(int col, int row, CellType type);

    const std::vector<Waypoint>& getWaypoints() const { return waypoints_; }
    MapType getType() const { return type_; }

    bool canPlaceTower(int col, int row) const;
    void placeTower(int col, int row);
    
    void setMapType(MapType type);
    const char* getMapName() const;

private:
    CellType grid_[COLS][ROWS];
    std::vector<Waypoint> waypoints_;
    MapType type_;

    void initMap();
    void initWaypoints();
    void initForest();
    void initDesert();
    void initSnow();
    void initDungeon();
};

#endif //TOWERDEFENSE_MAP_H
