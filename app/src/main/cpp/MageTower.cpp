#include "MageTower.h"
#include "Projectile.h"
#include "Game.h"
#include <cmath>
#include <cfloat>
#include <algorithm>

int MageTower::getCost(Difficulty diff) {
    switch (diff) {
        case Difficulty::HARD: return 200;
        default: return 250;
    }
}

MageTower::MageTower(int gridCol, int gridRow, ElementType element)
    : gridCol_(gridCol), gridRow_(gridRow), cooldown_(0.0f), element_(element) {
    x_ = gridCol + 0.5f;
    y_ = gridRow + 0.5f;
    recalculateStats();
}

MageConfig MageTower::getConfig() const {
    MageConfig config;
    config.element = element_;
    config.range = 2.5f + (level_ - 1) * 0.2f;
    config.fireRate = 0.8f + (level_ - 1) * 0.1f; // Медленнее чем Archer
    config.aoeRadius = 1.0f + (level_ - 1) * 0.15f;
    
    switch (element_) {
        case ElementType::FIRE:
            config.damage = 15 + (level_ - 1) * 8;
            config.dotDuration = 3.0f + (level_ - 1) * 0.5f;
            config.dotDamage = 5 + (level_ - 1) * 3;
            break;
            
        case ElementType::ICE:
            config.damage = 12 + (level_ - 1) * 6;
            config.slowFactor = 0.5f - (level_ - 1) * 0.05f; // 50% -> 30% скорости
            config.slowDuration = 2.0f + (level_ - 1) * 0.3f;
            break;
            
        case ElementType::LIGHTNING:
            config.damage = 20 + (level_ - 1) * 10;
            config.chainTargets = 2 + (level_ - 1); // 2 -> 6 целей
            break;
    }
    
    return config;
}

void MageTower::update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    if (cooldown_ > 0.0f) {
        cooldown_ -= dt;
    }
    
    if (cooldown_ <= 0.0f) {
        int targetIdx = findTarget(enemies);
        if (targetIdx >= 0) {
            MageConfig config = getConfig();
            
            // For lightning, we handle it specially
            if (config.element == ElementType::LIGHTNING) {
                applyChainLightning(enemies, targetIdx, config.damage, config.chainTargets);
            } else {
                // Fire and Ice apply AoE at target location
                applyAoeDamage(enemies, targetIdx, config.damage, config);
            }
            
            // Visual projectile (even though damage is instant)
            projectiles.emplace_back(x_, y_, targetIdx, 0); // 0 damage, visual only
            
            cooldown_ = 1.0f / config.fireRate;
        }
    }
}

void MageTower::applyAoeDamage(std::vector<Enemy>& enemies, int targetIdx, float damage, const MageConfig& config) {
    if (targetIdx < 0 || targetIdx >= (int)enemies.size()) return;
    
    float targetX = enemies[targetIdx].getX();
    float targetY = enemies[targetIdx].getY();
    
    for (auto& enemy : enemies) {
        if (!enemy.isAlive()) continue;
        
        float dx = enemy.getX() - targetX;
        float dy = enemy.getY() - targetY;
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist <= config.aoeRadius) {
            // Check combo: Frozen + Fire = x2 damage
            bool wasFrozen = enemy.isFrozen();
            float finalDamage = damage;
            
            if (config.element == ElementType::FIRE && wasFrozen) {
                finalDamage *= 2.0f; // COMBO!
                enemy.clearStatusEffects(); // Fire removes ice
            }
            
            enemy.takeDamage(finalDamage);
            
            // Apply element effects
            switch (config.element) {
                case ElementType::FIRE:
                    enemy.applyBurn(config.dotDamage, config.dotDuration);
                    break;
                    
                case ElementType::ICE:
                    enemy.applySlow(config.slowFactor, config.slowDuration);
                    break;
                    
                default:
                    break;
            }
        }
    }
}

void MageTower::applyChainLightning(std::vector<Enemy>& enemies, int firstTargetIdx, float damage, int chainCount) {
    if (firstTargetIdx < 0 || firstTargetIdx >= (int)enemies.size()) return;
    if (!enemies[firstTargetIdx].isAlive()) return;
    
    std::vector<int> hitIndices;
    hitIndices.push_back(firstTargetIdx);
    
    // Mark first target as hit
    enemies[firstTargetIdx].takeDamage(damage);
    
    // Chain to nearest targets
    for (int chain = 1; chain < chainCount; ++chain) {
        int lastHit = hitIndices.back();
        float lastX = enemies[lastHit].getX();
        float lastY = enemies[lastHit].getY();
        
        // Find nearest unhit enemy
        int nearestIdx = -1;
        float nearestDist = FLT_MAX;
        
        for (int i = 0; i < (int)enemies.size(); ++i) {
            if (!enemies[i].isAlive()) continue;
            if (std::find(hitIndices.begin(), hitIndices.end(), i) != hitIndices.end()) continue;
            
            float dx = enemies[i].getX() - lastX;
            float dy = enemies[i].getY() - lastY;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist < nearestDist && dist <= 3.0f) { // Max chain range
                nearestDist = dist;
                nearestIdx = i;
            }
        }
        
        if (nearestIdx >= 0) {
            enemies[nearestIdx].takeDamage(damage * 0.75f); // Each bounce does less damage
            hitIndices.push_back(nearestIdx);
        } else {
            break; // No more targets in range
        }
    }
}

int MageTower::findTarget(const std::vector<Enemy>& enemies) const {
    MageConfig config = getConfig();
    int bestIndex = -1;
    float bestProgress = -1.0f;
    
    for (int i = 0; i < (int)enemies.size(); ++i) {
        const Enemy& e = enemies[i];
        if (!e.isAlive() || e.reachedEnd()) continue;
        
        float dx = e.getX() - x_;
        float dy = e.getY() - y_;
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist <= config.range && e.getPathProgress() > bestProgress) {
            bestProgress = e.getPathProgress();
            bestIndex = i;
        }
    }
    
    return bestIndex;
}

void MageTower::recalculateStats() {
    // Stats are calculated on-demand in getConfig()
}

int MageTower::getUpgradeCost() const {
    // Same progression as Archer: 30, 50, 100, 200
    switch (level_) {
        case 1: return 30;
        case 2: return 50;
        case 3: return 100;
        case 4: return 200;
        default: return 0;
    }
}

void MageTower::upgrade() {
    if (level_ < getMaxLevel()) {
        level_++;
        recalculateStats();
    }
}

void MageTower::setElement(ElementType element) {
    element_ = element;
    recalculateStats();
}
