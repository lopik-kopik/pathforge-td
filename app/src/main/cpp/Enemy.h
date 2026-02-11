#ifndef TOWERDEFENSE_ENEMY_H
#define TOWERDEFENSE_ENEMY_H

#include <vector>
#include "Map.h"
#include "SpriteSheet.h"

enum class EnemyType {
    SLIME = 0,
    GOBLIN = 1,
    BOSS = 2,
    BAT = 3,        // Летающий
    FLYING_EYE = 4  // Летающий
};

enum class EnemyState {
    NORMAL,
    BURNING,
    FROZEN,
    SLOWED
};;

class Enemy {
public:
    Enemy(EnemyType type, const std::vector<Waypoint>& waypoints);

    void update(float dt);

    bool isAlive() const { return hp_ > 0; }
    bool isDying() const { return isDying_; }
    bool isDead() const { return isDead_; }
    bool reachedEnd() const { return reachedEnd_; }
    
    void startDeath();
    float getDeathProgress() const { return deathAnimTimer_ / deathAnimDuration_; }

    void takeDamage(int damage);

    float getX() const { return x_; }
    float getY() const { return y_; }
    float getHPRatio() const { return isDying_ ? 0.0f : (float)hp_ / (float)maxHP_; }
    int getReward() const { return reward_; }
    bool isRewardGranted() const { return rewardGranted_; }
    void markRewardGranted() { rewardGranted_ = true; }
    EnemyType getType() const { return type_; }

    int getAnimFrame() const { return animFrame_; }
    int getDeathFrame() const { return deathFrame_; }
    
    // Returns progress along path from 0.0 (start) to 1.0 (end)
    float getPathProgress() const;
    
    // Apply difficulty multiplier (percentage, e.g., 75 for 75%)
    void applyDifficulty(int multiplier);
    
    // Elemental effects
    void applyBurn(float damagePerSec, float duration);
    void applySlow(float factor, float duration);
    void freeze(float duration);
    bool isFrozen() const { return frozenTimer_ > 0.0f; }
    bool isSlowed() const { return slowTimer_ > 0.0f; }
    bool isBurning() const { return burnTimer_ > 0.0f; }
    void clearStatusEffects();
    
    // Flying enemies
    bool isFlying() const { return type_ == EnemyType::BAT || type_ == EnemyType::FLYING_EYE; }
    
    // Get current speed (affected by slow)
    float getCurrentSpeed() const;

private:
    EnemyType type_;
    float x_, y_;
    float speed_;
    int hp_;
    int maxHP_;
    int reward_;
    bool reachedEnd_;
    bool rewardGranted_ = false;

    const std::vector<Waypoint>* waypoints_;
    int currentWaypoint_;

    float animTimer_;
    int animFrame_;
    int animFrameCount_;
    static constexpr float ANIM_FRAME_TIME = 0.15f;
    
    // Death animation
    bool isDying_ = false;
    bool isDead_ = false;
    float deathAnimTimer_ = 0.0f;
    static constexpr float deathAnimDuration_ = 0.6f; // 6 frames * 0.1s
    int deathFrame_ = 0;
    static constexpr float DEATH_FRAME_TIME = 0.1f;
    
    // Status effects
    float burnTimer_ = 0.0f;
    float burnDamagePerSec_ = 0.0f;
    float burnTickTimer_ = 0.0f;
    
    float slowTimer_ = 0.0f;
    float slowFactor_ = 1.0f; // 1.0 = normal speed
    
    float frozenTimer_ = 0.0f;
    
    // Visual effect
    EnemyState visualState_ = EnemyState::NORMAL;
    float stateFlashTimer_ = 0.0f;
};

#endif //TOWERDEFENSE_ENEMY_H
