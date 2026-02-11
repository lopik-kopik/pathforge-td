#ifndef TOWERDEFENSE_PROJECTILE_H
#define TOWERDEFENSE_PROJECTILE_H

class Projectile {
public:
    Projectile(float startX, float startY, int targetEnemyIndex, int damage);

    void update(float dt, float targetX, float targetY);

    bool isActive() const { return active_; }
    void deactivate() { active_ = false; }

    float getX() const { return x_; }
    float getY() const { return y_; }
    int getTargetIndex() const { return targetIndex_; }
    int getDamage() const { return damage_; }

    bool hasReachedTarget(float targetX, float targetY) const;

private:
    float x_, y_;
    int targetIndex_;
    int damage_;
    float speed_;
    bool active_;
};

#endif //TOWERDEFENSE_PROJECTILE_H
