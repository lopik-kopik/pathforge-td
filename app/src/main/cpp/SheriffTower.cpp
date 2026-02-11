#include "SheriffTower.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Ally.h"
#include "Map.h"
#include "Game.h"
#include <cmath>
#include <cfloat>

int SheriffTower::getCost(Difficulty diff) {
    switch (diff) {
        case Difficulty::HARD: return 150;
        default: return 200;
    }
}

SheriffTower::SheriffTower(int gridCol, int gridRow)
    : gridCol_(gridCol), gridRow_(gridRow), fireCooldown_(0.0f), spawnCooldown_(SPAWN_RATE) {
    x_ = gridCol + 0.5f;
    y_ = gridRow + 0.5f;
}

void SheriffTower::update(float dt, const std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles,
                          const std::vector<Waypoint>& waypoints, std::vector<Ally>& allies) {
    // Fire at enemies
    if (fireCooldown_ > 0.0f) {
        fireCooldown_ -= dt;
    }

    if (fireCooldown_ <= 0.0f) {
        int targetIdx = findTarget(enemies);
        if (targetIdx >= 0) {
            projectiles.emplace_back(x_, y_, targetIdx, DAMAGE);
            fireCooldown_ = 1.0f / FIRE_RATE;
        }
    }
    
    // Spawn ally
    spawnCooldown_ -= dt;
    if (spawnCooldown_ <= 0.0f && !waypoints.empty()) {
        // Spawn at the base (last waypoint)
        allies.emplace_back(waypoints.back().x, waypoints.back().y, waypoints);
        spawnCooldown_ = SPAWN_RATE;
    }
}

int SheriffTower::findTarget(const std::vector<Enemy>& enemies) const {
    int bestIndex = -1;
    float bestProgress = -1.0f;

    for (int i = 0; i < (int)enemies.size(); i++) {
        const Enemy& e = enemies[i];
        // Sheriff cannot target flying enemies
        if (e.isFlying()) continue;
        if (!e.isAlive() || e.reachedEnd()) continue;

        float dx = e.getX() - x_;
        float dy = e.getY() - y_;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= RANGE && e.getPathProgress() > bestProgress) {
            bestProgress = e.getPathProgress();
            bestIndex = i;
        }
    }

    return bestIndex;
}
