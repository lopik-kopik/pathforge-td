#ifndef TOWERDEFENSE_MAGETOWER_H
#define TOWERDEFENSE_MAGETOWER_H

#include <vector>
#include "Enemy.h"

class Projectile;
enum class Difficulty;

enum class ElementType {
    FIRE,   // Урон со временем (DoT)
    ICE,    // Замедление
    LIGHTNING // Цепной урон
};

struct MageConfig {
    ElementType element;
    float damage;
    float range;
    float fireRate;
    float aoeRadius;      // Радиус взрыва
    // Специфичные параметры
    float dotDuration;    // Длительность горения (FIRE)
    float dotDamage;      // Урон от горения в сек (FIRE)
    float slowFactor;     // Коэффициент замедления 0-1 (ICE)
    float slowDuration;   // Длительность замедления (ICE)
    int chainTargets;     // Количество целей для цепи (LIGHTNING)
};

class MageTower {
public:
    MageTower(int gridCol, int gridRow, ElementType element = ElementType::FIRE);

    void update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);

    float getX() const { return x_; }
    float getY() const { return y_; }
    int getGridCol() const { return gridCol_; }
    int getGridRow() const { return gridRow_; }
    ElementType getElement() const { return element_; }
    
    static int getCost() { return 250; }
    static int getCost(Difficulty diff);
    
    // Upgrade system
    int getLevel() const { return level_; }
    int getUpgradeCost() const;
    bool canUpgrade(int playerGold) const { return level_ < getMaxLevel() && playerGold >= getUpgradeCost(); }
    void upgrade();
    static int getMaxLevel() { return 5; }
    
    // Change element (costs cards)
    void setElement(ElementType element);

private:
    int gridCol_, gridRow_;
    float x_, y_;
    float cooldown_;
    int level_ = 1;
    ElementType element_;
    
    void recalculateStats();
    MageConfig getConfig() const;
    
    int findTarget(const std::vector<Enemy>& enemies) const;
    void applyAoeDamage(std::vector<Enemy>& enemies, int targetIdx, float damage, const MageConfig& config);
    
    // Lightning chain logic
    void applyChainLightning(std::vector<Enemy>& enemies, int firstTargetIdx, float damage, int chainCount);
};

#endif //TOWERDEFENSE_MAGETOWER_H
