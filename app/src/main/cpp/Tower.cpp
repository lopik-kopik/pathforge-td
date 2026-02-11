#include "Tower.h"
#include "Enemy.h"
#include "Game.h"
#include <cmath>
#include <cfloat>

int Tower::getCost(Difficulty diff) {
    switch (diff) {
        case Difficulty::HARD: return 35;
        default: return 50;
    }
}

Tower::Tower(int gridCol, int gridRow)
    : gridCol_(gridCol), gridRow_(gridRow),
      damage_(10), range_(2.5f), fireRate_(1.0f), cooldown_(0.0f) {
    // World position is center of cell
    x_ = gridCol + 0.5f;
    y_ = gridRow + 0.5f;
}

void Tower::update(float dt) {
    if (cooldown_ > 0.0f) {
        cooldown_ -= dt;
    }
}

int Tower::findTarget(const std::vector<Enemy>& enemies) const {
    int bestIndex = -1;
    float bestProgress = -1.0f; // Target enemy furthest along path (last enemy)

    for (int i = 0; i < (int)enemies.size(); i++) {
        const Enemy& e = enemies[i];
        if (!e.isAlive() || e.reachedEnd()) continue;

        float dx = e.getX() - x_;
        float dy = e.getY() - y_;
        float dist = std::sqrt(dx * dx + dy * dy);

        // Archer can hit both ground and flying enemies
        if (dist <= range_ && e.getPathProgress() > bestProgress) {
            bestProgress = e.getPathProgress();
            bestIndex = i;
        }
    }

    return bestIndex;
}

int Tower::getUpgradeCost() const {
    // Cost: 30, 50, 100, 200, 250
    switch (level_) {
        case 1: return 30;
        case 2: return 50;
        case 3: return 100;
        case 4: return 200;
        default: return 0;
    }
}

void Tower::upgrade() {
    if (level_ < getMaxLevel()) {
        level_++;
        recalculateStats();
    }
}

void Tower::recalculateStats() {
    // Base stats at level 1: damage=10, range=2.5, fireRate=1.0
    // Each level: +5 damage, +0.3 range, +0.2 fireRate
    damage_ = 10 + (level_ - 1) * 5;
    range_ = 2.5f + (level_ - 1) * 0.3f;
    fireRate_ = 1.0f + (level_ - 1) * 0.2f;
}
