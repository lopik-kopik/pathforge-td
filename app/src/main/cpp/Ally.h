#ifndef TOWERDEFENSE_ALLY_H
#define TOWERDEFENSE_ALLY_H

#include <vector>
#include "Map.h"

// Ally unit spawned by Sheriff tower
// Moves from base to spawn (reverse direction) and fights enemies
class Game; // Forward declaration

class Ally {
public:
    Ally(float startX, float startY, const std::vector<Waypoint>& waypoints);

    void update(float dt, std::vector<class Enemy>& enemies, Game* game);

    bool isAlive() const { return hp_ > 0; }
    bool reachedSpawn() const { return reachedSpawn_; }

    float getX() const { return x_; }
    float getY() const { return y_; }
    int getDamage() const { return damage_; }
    int getHP() const { return hp_; }
    int getMaxHP() const { return maxHP_; }

    void takeDamage(int damage);

private:
    float x_, y_;
    int hp_;
    int maxHP_;
    int damage_;
    float speed_;
    bool reachedSpawn_;

    const std::vector<Waypoint>* waypoints_;
    int currentWaypoint_; // Goes backwards from end to start

    // Attack cooldown
    float attackCooldown_;
    static constexpr float ATTACK_RANGE = 0.5f;
    static constexpr float ATTACK_COOLDOWN = 1.0f; // 1 attack per second
};

#endif //TOWERDEFENSE_ALLY_H
