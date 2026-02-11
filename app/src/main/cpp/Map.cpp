#include "Map.h"

Map::Map() : type_(MapType::FOREST) {
    initMap();
    initWaypoints();
}

Map::Map(MapType type) : type_(type) {
    initMap();
    initWaypoints();
}

void Map::setMapType(MapType type) {
    type_ = type;
    initMap();
    initWaypoints();
}

const char* Map::getMapName() const {
    switch (type_) {
        case MapType::FOREST: return "Forest";
        case MapType::DESERT: return "Desert";
        case MapType::SNOW: return "Snow";
        case MapType::DUNGEON: return "Dungeon";
        default: return "Unknown";
    }
}

void Map::initMap() {
    switch (type_) {
        case MapType::DESERT:
            initDesert();
            break;
        case MapType::SNOW:
            initSnow();
            break;
        case MapType::DUNGEON:
            initDungeon();
            break;
        default:
            initForest();
            break;
    }
}

void Map::initForest() {
    // Fill with grass
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS; r++) {
            grid_[c][r] = CellType::GRASS;
        }
    }

    // Classic S-shaped path
    int pathCells[][2] = {
        {5, 15}, {5, 14}, {5, 13}, {5, 12}, {5, 11},
        {6, 11}, {7, 11}, {8, 11},
        {8, 10}, {8, 9}, {8, 8},
        {7, 8}, {6, 8}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8},
        {1, 7}, {1, 6}, {1, 5},
        {2, 5}, {3, 5}, {4, 5}, {5, 5}, {6, 5}, {7, 5},
        {7, 4}, {7, 3}, {7, 2},
        {6, 2}, {5, 2}, {4, 2},
        {4, 1}, {4, 0},
    };

    int pathCount = sizeof(pathCells) / sizeof(pathCells[0]);
    for (int i = 0; i < pathCount; i++) {
        grid_[pathCells[i][0]][pathCells[i][1]] = CellType::PATH;
    }

    // Trees as blocked cells
    int blockedCells[][2] = {
        {0, 15}, {1, 15}, {9, 15},
        {0, 12}, {3, 13}, {9, 13},
        {0, 9}, {3, 10}, {6, 13},
        {9, 6}, {9, 3}, {0, 3},
        {0, 0}, {1, 0}, {9, 0},
    };
    int blockedCount = sizeof(blockedCells) / sizeof(blockedCells[0]);
    for (int i = 0; i < blockedCount; i++) {
        grid_[blockedCells[i][0]][blockedCells[i][1]] = CellType::BLOCKED;
    }
}

void Map::initDesert() {
    // Fill with sand (grass texture reused)
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS; r++) {
            grid_[c][r] = CellType::GRASS;
        }
    }

    // Zigzag path
    int pathCells[][2] = {
        {2, 15}, {2, 14}, {2, 13}, {2, 12},
        {3, 12}, {4, 12}, {5, 12}, {6, 12}, {7, 12},
        {7, 11}, {7, 10}, {7, 9},
        {6, 9}, {5, 9}, {4, 9}, {3, 9}, {2, 9},
        {2, 8}, {2, 7}, {2, 6},
        {3, 6}, {4, 6}, {5, 6}, {6, 6}, {7, 6},
        {7, 5}, {7, 4}, {7, 3},
        {6, 3}, {5, 3}, {4, 3}, {3, 3}, {2, 3},
        {2, 2}, {2, 1}, {2, 0},
    };

    int pathCount = sizeof(pathCells) / sizeof(pathCells[0]);
    for (int i = 0; i < pathCount; i++) {
        grid_[pathCells[i][0]][pathCells[i][1]] = CellType::PATH;
    }

    // Cacti/rocks
    int blockedCells[][2] = {
        {0, 0}, {1, 0}, {9, 0}, {8, 0},
        {0, 15}, {1, 15}, {9, 15}, {8, 15},
        {0, 7}, {9, 7}, {4, 14}, {5, 1},
    };
    int blockedCount = sizeof(blockedCells) / sizeof(blockedCells[0]);
    for (int i = 0; i < blockedCount; i++) {
        grid_[blockedCells[i][0]][blockedCells[i][1]] = CellType::BLOCKED;
    }
}

void Map::initSnow() {
    // Fill with snow
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS; r++) {
            grid_[c][r] = CellType::GRASS;
        }
    }

    // Spiral path
    int pathCells[][2] = {
        {5, 15}, {5, 14}, {5, 13},
        {4, 13}, {3, 13}, {2, 13}, {1, 13},
        {1, 12}, {1, 11}, {1, 10},
        {2, 10}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {7, 10}, {8, 10},
        {8, 9}, {8, 8}, {8, 7},
        {7, 7}, {6, 7}, {5, 7}, {4, 7}, {3, 7}, {2, 7},
        {2, 6}, {2, 5}, {2, 4},
        {3, 4}, {4, 4}, {5, 4}, {6, 4}, {7, 4},
        {7, 3}, {7, 2}, {7, 1},
        {6, 1}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 1},
        {1, 0},
    };

    int pathCount = sizeof(pathCells) / sizeof(pathCells[0]);
    for (int i = 0; i < pathCount; i++) {
        grid_[pathCells[i][0]][pathCells[i][1]] = CellType::PATH;
    }

    // Snowmen/rocks
    int blockedCells[][2] = {
        {0, 0}, {9, 0}, {9, 15}, {0, 15},
        {3, 14}, {7, 14}, {4, 5}, {6, 5},
    };
    int blockedCount = sizeof(blockedCells) / sizeof(blockedCells[0]);
    for (int i = 0; i < blockedCount; i++) {
        grid_[blockedCells[i][0]][blockedCells[i][1]] = CellType::BLOCKED;
    }
}

void Map::initDungeon() {
    // Fill with dark floor
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS; r++) {
            grid_[c][r] = CellType::GRASS;
        }
    }

    // Double loop path
    int pathCells[][2] = {
        {1, 15}, {1, 14}, {1, 13}, {1, 12}, {1, 11}, {1, 10},
        {2, 10}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {7, 10}, {8, 10},
        {8, 9}, {8, 8}, {8, 7}, {8, 6},
        {7, 6}, {6, 6}, {5, 6}, {4, 6}, {3, 6}, {2, 6},
        {2, 5}, {2, 4},
        {3, 4}, {4, 4}, {5, 4}, {6, 4}, {7, 4}, {8, 4},
        {8, 3}, {8, 2}, {8, 1},
        {7, 1}, {6, 1}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 1},
        {1, 0},
    };

    int pathCount = sizeof(pathCells) / sizeof(pathCells[0]);
    for (int i = 0; i < pathCount; i++) {
        grid_[pathCells[i][0]][pathCells[i][1]] = CellType::PATH;
    }

    // Rocks/walls
    int blockedCells[][2] = {
        {0, 0}, {9, 0}, {9, 15}, {0, 15},
        {3, 8}, {6, 8}, {4, 2}, {5, 2},
        {0, 5}, {9, 5}, {0, 10}, {9, 10},
    };
    int blockedCount = sizeof(blockedCells) / sizeof(blockedCells[0]);
    for (int i = 0; i < blockedCount; i++) {
        grid_[blockedCells[i][0]][blockedCells[i][1]] = CellType::BLOCKED;
    }
}

void Map::initWaypoints() {
    waypoints_.clear();
    
    // Find path cells and add as waypoints
    // Simple approach: scan grid and add path cells in order
    // This works because we define paths in order
    
    // For each map type, we manually define waypoints
    switch (type_) {
        case MapType::DESERT:
            waypoints_ = {
                {2.5f, 15.5f}, {2.5f, 12.5f}, {7.5f, 12.5f}, {7.5f, 9.5f},
                {2.5f, 9.5f}, {2.5f, 6.5f}, {7.5f, 6.5f}, {7.5f, 3.5f},
                {2.5f, 3.5f}, {2.5f, 0.5f}
            };
            break;
        case MapType::SNOW:
            waypoints_ = {
                {5.5f, 15.5f}, {5.5f, 13.5f}, {1.5f, 13.5f}, {1.5f, 10.5f},
                {8.5f, 10.5f}, {8.5f, 7.5f}, {2.5f, 7.5f}, {2.5f, 4.5f},
                {7.5f, 4.5f}, {7.5f, 1.5f}, {1.5f, 1.5f}, {1.5f, 0.5f}
            };
            break;
        case MapType::DUNGEON:
            waypoints_ = {
                {1.5f, 15.5f}, {1.5f, 10.5f}, {8.5f, 10.5f}, {8.5f, 6.5f},
                {2.5f, 6.5f}, {2.5f, 4.5f}, {8.5f, 4.5f}, {8.5f, 1.5f},
                {1.5f, 1.5f}, {1.5f, 0.5f}
            };
            break;
        default: // FOREST
            waypoints_ = {
                {5.5f, 15.5f}, {5.5f, 11.5f}, {8.5f, 11.5f}, {8.5f, 8.5f},
                {1.5f, 8.5f}, {1.5f, 5.5f}, {7.5f, 5.5f}, {7.5f, 2.5f},
                {4.5f, 2.5f}, {4.5f, 0.5f}
            };
            break;
    }
}

CellType Map::getCell(int col, int row) const {
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS) {
        return CellType::BLOCKED;
    }
    return grid_[col][row];
}

void Map::setCell(int col, int row, CellType type) {
    if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
        grid_[col][row] = type;
    }
}

bool Map::canPlaceTower(int col, int row) const {
    return getCell(col, row) == CellType::GRASS;
}

void Map::placeTower(int col, int row) {
    setCell(col, row, CellType::TOWER);
}
