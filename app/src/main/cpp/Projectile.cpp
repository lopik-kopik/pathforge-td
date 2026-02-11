#include "Projectile.h"
#include <cmath>

Projectile::Projectile(float startX, float startY, int targetEnemyIndex, int damage)
    : x_(startX), y_(startY), targetIndex_(targetEnemyIndex),
      damage_(damage), speed_(8.0f), active_(true) {
}

void Projectile::update(float dt, float targetX, float targetY) {
    if (!active_) return;

    float dx = targetX - x_;
    float dy = targetY - y_;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 0.01f) {
        x_ = targetX;
        y_ = targetY;
        return;
    }

    float moveAmount = speed_ * dt;
    if (moveAmount > dist) moveAmount = dist;

    x_ += (dx / dist) * moveAmount;
    y_ += (dy / dist) * moveAmount;
}

bool Projectile::hasReachedTarget(float targetX, float targetY) const {
    float dx = targetX - x_;
    float dy = targetY - y_;
    return (dx * dx + dy * dy) < 0.2f * 0.2f;
}
