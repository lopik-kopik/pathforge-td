#ifndef TOWERDEFENSE_WAVE_H
#define TOWERDEFENSE_WAVE_H

#include <vector>
#include "Enemy.h"

struct WaveEntry {
    EnemyType enemyType;
    int count;
};

struct Wave {
    std::vector<WaveEntry> entries;
    float spawnInterval;
    int difficultyMultiplier = 100; // HP multiplier in percent (100 = normal)
};

class WaveManager {
public:
    WaveManager();

    const Wave& getCurrentWave() const;
    int getCurrentWaveIndex() const { return currentWave_; }
    int getTotalWaves() const { return (int)waves_.size(); }
    bool allWavesDone() const { return currentWave_ >= (int)waves_.size(); }

    void advanceWave();
    
    // Generate endless wave based on wave number
    Wave generateEndlessWave(int waveNumber) const;

private:
    std::vector<Wave> waves_;
    int currentWave_;
};

#endif //TOWERDEFENSE_WAVE_H
