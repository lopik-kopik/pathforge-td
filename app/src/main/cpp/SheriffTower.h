#ifndef TOWERDEFENSE_SHERIFFTOWER_H
#define TOWERDEFENSE_SHERIFFTOWER_H

#include <vector>
#include "Map.h"

class Enemy;
class Ally;
enum class Difficulty;

class Projectile;

class SheriffTower {
public:
    SheriffTower(int gridCol, int gridRow);

    void update(float dt, const std::vector<class Enemy>& enemies, std::vector<Projectile>& projectiles,
                const std::vector<Waypoint>& waypoints, std::vector<Ally>& allies);

    float getX() const { return x_; }
    float getY() const { return y_; }
    int getGridCol() const { return gridCol_; }
    int getGridRow() const { return gridRow_; }

    static int getCost() { return 200; }
    static int getCost(Difficulty diff);

private:
    int gridCol_, gridRow_;
    float x_, y_;
    float fireCooldown_;
    float spawnCooldown_;

    static constexpr float FIRE_RATE = 1.0f; // 1 shot per second
    static constexpr float SPAWN_RATE = 15.0f; // 1 ally per 15 seconds
    static constexpr float RANGE = 3.0f;
    static constexpr int DAMAGE = 30;
    
    int findTarget(const std::vector<class Enemy>& enemies) const;
};

#endif //TOWERDEFENSE_SHERIFFTOWER_H
