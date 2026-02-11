#include "Ally.h"
#include "Enemy.h"
#include "Game.h"
#include <cmath>

Ally::Ally(float startX, float startY, const std::vector<Waypoint>& waypoints)
    : x_(startX), y_(startY), hp_(200), maxHP_(200), damage_(60),
      speed_(1.0f), reachedSpawn_(false), waypoints_(&waypoints),
      currentWaypoint_((int)waypoints.size() - 2), attackCooldown_(0.0f) {
    // Start at base (last waypoint), move towards spawn (first waypoint)
}

void Ally::update(float dt, std::vector<Enemy>& enemies, Game* game) {
    if (!isAlive() || reachedSpawn_) return;

    // Update attack cooldown
    if (attackCooldown_ > 0.0f) {
        attackCooldown_ -= dt;
    }

    // Find enemy to attack
    int targetIndex = -1;
    float bestDist = ATTACK_RANGE;

    for (int i = 0; i < (int)enemies.size(); i++) {
        if (!enemies[i].isAlive()) continue;
        float dx = enemies[i].getX() - x_;
        float dy = enemies[i].getY() - y_;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < bestDist) {
            bestDist = dist;
            targetIndex = i;
        }
    }

    // Attack if in range
    if (targetIndex >= 0 && attackCooldown_ <= 0.0f) {
        enemies[targetIndex].takeDamage(damage_);
        
        // Spawn damage number
        if (game) {
            game->spawnDamageNumber(enemies[targetIndex].getX(), 
                                   enemies[targetIndex].getY() + 0.3f, 
                                   damage_);
        }
        
        // Death and gold reward now handled in Game::updateEnemies()
        attackCooldown_ = ATTACK_COOLDOWN;
        return; // Don't move while attacking
    }

    // Move towards current waypoint (backwards)
    if (currentWaypoint_ >= 0) {
        float targetX = (*waypoints_)[currentWaypoint_].x;
        float targetY = (*waypoints_)[currentWaypoint_].y;

        float dx = targetX - x_;
        float dy = targetY - y_;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 0.05f) {
            // Reached waypoint, move to next (backwards)
            x_ = targetX;
            y_ = targetY;
            currentWaypoint_--;

            if (currentWaypoint_ < 0) {
                reachedSpawn_ = true;
                return;
            }
        } else {
            // Move toward waypoint
            float moveAmount = speed_ * dt;
            if (moveAmount > dist) moveAmount = dist;
            x_ += (dx / dist) * moveAmount;
            y_ += (dy / dist) * moveAmount;
        }
    }
}

void Ally::takeDamage(int damage) {
    hp_ -= damage;
    if (hp_ < 0) hp_ = 0;
}
