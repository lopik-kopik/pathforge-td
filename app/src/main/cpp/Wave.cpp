#include "Wave.h"

WaveManager::WaveManager() : currentWave_(0) {
    // Wave 1: 3 slimes
    waves_.push_back({{{EnemyType::SLIME, 3}}, 1.5f});

    // Wave 2: 5 slimes
    waves_.push_back({{{EnemyType::SLIME, 5}}, 1.3f});

    // Wave 3: 3 goblins + 2 bats (first flying)
    waves_.push_back({{{EnemyType::GOBLIN, 3}, {EnemyType::BAT, 2}}, 1.2f});

    // Wave 4: 8 slimes
    waves_.push_back({{{EnemyType::SLIME, 8}}, 1.1f});

    // Wave 5: 5 goblins + 5 slimes + 3 bats
    waves_.push_back({{{EnemyType::GOBLIN, 5}, {EnemyType::SLIME, 5}, {EnemyType::BAT, 3}}, 1.0f});

    // Wave 6: 10 slimes + 5 bats
    waves_.push_back({{{EnemyType::SLIME, 10}, {EnemyType::BAT, 5}}, 0.9f});

    // Wave 7: 8 goblins + 3 flying eyes
    waves_.push_back({{{EnemyType::GOBLIN, 8}, {EnemyType::FLYING_EYE, 3}}, 0.9f});

    // Wave 8: 5 goblins + 10 slimes + 5 bats
    waves_.push_back({{{EnemyType::GOBLIN, 5}, {EnemyType::SLIME, 10}, {EnemyType::BAT, 5}}, 0.8f});

    // Wave 9: 12 goblins + 5 flying eyes
    waves_.push_back({{{EnemyType::GOBLIN, 12}, {EnemyType::FLYING_EYE, 5}}, 0.7f});

    // Wave 10: BOSS - 1 super goblin + flying support
    waves_.push_back({{{EnemyType::BOSS, 1}, {EnemyType::BAT, 10}}, 2.0f});
}

const Wave& WaveManager::getCurrentWave() const {
    return waves_[currentWave_];
}

void WaveManager::advanceWave() {
    currentWave_++;
}

Wave WaveManager::generateEndlessWave(int waveNumber) const {
    Wave wave;
    
    // Progressive difficulty:
    // Waves 1-5: Easy (75% HP)
    // Waves 6-10: Medium (100% HP)
    // Waves 11-20: Hard (150% HP)
    // Waves 21+: Super hard (200%+ HP, increases forever)
    int multiplier;
    if (waveNumber <= 5) {
        multiplier = 75;   // Easy
    } else if (waveNumber <= 10) {
        multiplier = 100;  // Medium
    } else if (waveNumber <= 20) {
        multiplier = 150;  // Hard
    } else {
        multiplier = 200 + (waveNumber - 20) * 10;  // Super hard: 210%, 220%, etc.
    }
    wave.difficultyMultiplier = multiplier;
    
    // Scale enemy count with wave number
    int baseCount = 5 + (waveNumber * 2);  // More enemies each wave
    float interval = std::max(0.3f, 1.0f - (waveNumber * 0.03f));  // Faster spawns
    
    // Every 5th wave has a boss
    if (waveNumber % 5 == 0) {
        wave.entries.push_back({EnemyType::BOSS, 1});
        wave.entries.push_back({EnemyType::GOBLIN, baseCount / 2});
        wave.entries.push_back({EnemyType::FLYING_EYE, baseCount / 3});
    }
    // Every 3rd wave is goblin heavy
    else if (waveNumber % 3 == 0) {
        wave.entries.push_back({EnemyType::GOBLIN, baseCount});
        wave.entries.push_back({EnemyType::SLIME, baseCount / 2});
        wave.entries.push_back({EnemyType::BAT, baseCount / 3});
    }
    // Mixed waves with flying enemies
    else {
        wave.entries.push_back({EnemyType::GOBLIN, baseCount / 2});
        wave.entries.push_back({EnemyType::SLIME, baseCount});
        // Add flying enemies starting from wave 3
        if (waveNumber >= 3) {
            if (waveNumber % 2 == 0) {
                wave.entries.push_back({EnemyType::BAT, baseCount / 3});
            } else {
                wave.entries.push_back({EnemyType::FLYING_EYE, baseCount / 4});
            }
        }
    }
    
    wave.spawnInterval = interval;
    return wave;
}
