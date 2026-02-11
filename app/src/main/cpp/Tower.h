#ifndef TOWERDEFENSE_TOWER_H
#define TOWERDEFENSE_TOWER_H

#include <vector>

class Enemy;
enum class Difficulty;

class Tower {
public:
    Tower(int gridCol, int gridRow);

    void update(float dt);

    // Returns index of target enemy, or -1 if no target
    int findTarget(const std::vector<Enemy>& enemies) const;

    bool canFire() const { return cooldown_ <= 0.0f; }
    void resetCooldown() { cooldown_ = 1.0f / fireRate_; }

    // Animation state
    bool isAttacking() const { return attackAnimTimer_ > 0.0f; }
    float getAttackAnimProgress() const { return attackAnimTimer_ / attackAnimDuration_; }
    void triggerAttackAnim() { attackAnimTimer_ = attackAnimDuration_; }
    void updateAnim(float dt) { if (attackAnimTimer_ > 0.0f) attackAnimTimer_ -= dt; }

    float getX() const { return x_; }
    float getY() const { return y_; }
    int getDamage() const { return damage_; }
    float getRange() const { return range_; }
    float getFireRate() const { return fireRate_; }
    int getGridCol() const { return gridCol_; }
    int getGridRow() const { return gridRow_; }
    int getLevel() const { return level_; }

    // Upgrade system
    static int getCost() { return 50; }
    static int getCost(Difficulty diff);
    static int getMaxLevel() { return 5; }
    int getUpgradeCost() const;
    bool canUpgrade(int playerGold) const { return level_ < getMaxLevel() && playerGold >= getUpgradeCost(); }
    void upgrade();
    bool isUpgraded() const { return level_ >= 3; } // Visual change at level 3+

private:
    int gridCol_, gridRow_;
    float x_, y_; // world position (center of cell)
    int damage_;
    float range_;
    float fireRate_; // shots per second
    float cooldown_;
    int level_ = 1;
    
    // Attack animation
    float attackAnimTimer_ = 0.0f;
    static constexpr float attackAnimDuration_ = 0.3f; // seconds
    
    void recalculateStats();
};

#endif //TOWERDEFENSE_TOWER_H
