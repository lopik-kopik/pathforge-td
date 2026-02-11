#include "Enemy.h"
#include <cmath>

Enemy::Enemy(EnemyType type, const std::vector<Waypoint>& waypoints)
    : type_(type), waypoints_(&waypoints), currentWaypoint_(1),
      reachedEnd_(false), animTimer_(0), animFrame_(0) {

    // Start at first waypoint
    if (!waypoints_->empty()) {
        x_ = (*waypoints_)[0].x;
        y_ = (*waypoints_)[0].y;
    } else {
        x_ = 0;
        y_ = 0;
    }

    switch (type) {
        case EnemyType::SLIME:
            hp_ = 40;
            maxHP_ = 40;
            speed_ = 1.5f;
            reward_ = 10;
            animFrameCount_ = 6;
            break;
        case EnemyType::GOBLIN:
            hp_ = 90;
            maxHP_ = 90;
            speed_ = 1.0f;
            reward_ = 25;
            animFrameCount_ = 6;
            break;
        case EnemyType::BOSS:
            hp_ = 1500;
            maxHP_ = 1500;
            speed_ = 0.6f;
            reward_ = 100;
            animFrameCount_ = 6;
            break;
        case EnemyType::BAT:
            hp_ = 25;
            maxHP_ = 25;
            speed_ = 2.0f;
            reward_ = 15;
            animFrameCount_ = 4;
            break;
        case EnemyType::FLYING_EYE:
            hp_ = 60;
            maxHP_ = 60;
            speed_ = 1.3f;
            reward_ = 30;
            animFrameCount_ = 4;
            break;
    }
}

void Enemy::update(float dt) {
    // Update death animation
    if (isDying_) {
        deathAnimTimer_ += dt;
        deathFrame_ = (int)(deathAnimTimer_ / DEATH_FRAME_TIME);
        if (deathAnimTimer_ >= deathAnimDuration_) {
            isDead_ = true;
        }
        return;
    }
    
    if (!isAlive() || reachedEnd_) return;
    
    // Update status effects
    // Burn damage
    if (burnTimer_ > 0.0f) {
        burnTimer_ -= dt;
        burnTickTimer_ -= dt;
        if (burnTickTimer_ <= 0.0f) {
            takeDamage((int)burnDamagePerSec_);
            burnTickTimer_ = 1.0f; // Tick every second
        }
        visualState_ = EnemyState::BURNING;
    }
    
    // Slow effect
    if (slowTimer_ > 0.0f) {
        slowTimer_ -= dt;
        visualState_ = EnemyState::SLOWED;
    } else {
        slowFactor_ = 1.0f;
    }
    
    // Frozen effect
    if (frozenTimer_ > 0.0f) {
        frozenTimer_ -= dt;
        visualState_ = EnemyState::FROZEN;
        // When frozen, can't move
        dt = 0; // Skip movement
    }
    
    if (dt > 0 && currentWaypoint_ < (int)waypoints_->size()) {
        float targetX = (*waypoints_)[currentWaypoint_].x;
        float targetY = (*waypoints_)[currentWaypoint_].y;

        float dx = targetX - x_;
        float dy = targetY - y_;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 0.05f) {
            // Reached waypoint, move to next
            x_ = targetX;
            y_ = targetY;
            currentWaypoint_++;

            if (currentWaypoint_ >= (int)waypoints_->size()) {
                reachedEnd_ = true;
                return;
            }
        } else {
            // Move toward waypoint (affected by slow)
            float currentSpeed = speed_ * slowFactor_;
            if (frozenTimer_ > 0.0f) currentSpeed = 0.0f;
            
            float moveAmount = currentSpeed * dt;
            if (moveAmount > dist) moveAmount = dist;
            x_ += (dx / dist) * moveAmount;
            y_ += (dy / dist) * moveAmount;
        }
    }

    // Update animation
    animTimer_ += dt;
    if (animTimer_ >= ANIM_FRAME_TIME) {
        animTimer_ -= ANIM_FRAME_TIME;
        animFrame_ = (animFrame_ + 1) % animFrameCount_;
    }
}

void Enemy::takeDamage(int damage) {
    hp_ -= damage;
    if (hp_ < 0) hp_ = 0;
    // Auto-start death animation if killed
    if (hp_ == 0 && !isDying_ && !isDead_) {
        startDeath();
    }
}

void Enemy::startDeath() {
    if (!isDying_) {
        isDying_ = true;
        deathAnimTimer_ = 0.0f;
        deathFrame_ = 0;
    }
}

void Enemy::applyDifficulty(int multiplier) {
    // Adjust HP and speed based on difficulty
    hp_ = hp_ * multiplier / 100;
    maxHP_ = maxHP_ * multiplier / 100;
    speed_ = speed_ * multiplier / 100;
}

void Enemy::applyBurn(float damagePerSec, float duration) {
    burnDamagePerSec_ = damagePerSec;
    burnTimer_ = duration;
    burnTickTimer_ = 0.0f;
}

void Enemy::applySlow(float factor, float duration) {
    slowFactor_ = factor;
    slowTimer_ = duration;
}

void Enemy::freeze(float duration) {
    frozenTimer_ = duration;
}

void Enemy::clearStatusEffects() {
    burnTimer_ = 0.0f;
    slowTimer_ = 0.0f;
    slowFactor_ = 1.0f;
    frozenTimer_ = 0.0f;
    visualState_ = EnemyState::NORMAL;
}

float Enemy::getCurrentSpeed() const {
    if (frozenTimer_ > 0.0f) return 0.0f;
    return speed_ * slowFactor_;
}

float Enemy::getPathProgress() const {
    if (!waypoints_ || waypoints_->size() <= 1) return 0.0f;
    
    int totalSegments = (int)waypoints_->size() - 1;
    int completedSegments = currentWaypoint_ - 1;
    
    // Calculate progress within current segment
    float segmentProgress = 0.0f;
    if (currentWaypoint_ < (int)waypoints_->size()) {
        float targetX = (*waypoints_)[currentWaypoint_].x;
        float targetY = (*waypoints_)[currentWaypoint_].y;
        float dx = targetX - x_;
        float dy = targetY - y_;
        float distToNext = std::sqrt(dx * dx + dy * dy);
        
        float prevX = (*waypoints_)[currentWaypoint_ - 1].x;
        float prevY = (*waypoints_)[currentWaypoint_ - 1].y;
        float segmentDx = targetX - prevX;
        float segmentDy = targetY - prevY;
        float segmentLength = std::sqrt(segmentDx * segmentDx + segmentDy * segmentDy);
        
        if (segmentLength > 0.001f) {
            segmentProgress = 1.0f - (distToNext / segmentLength);
        }
    } else {
        completedSegments = totalSegments;
    }
    
    return (float)(completedSegments + segmentProgress) / (float)totalSegments;
}
