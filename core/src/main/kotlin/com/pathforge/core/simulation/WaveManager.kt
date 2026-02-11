package com.pathforge.core.simulation

import com.pathforge.core.domain.Difficulty

data class WaveConfig(
    val enemyCount: Int,
    val spawnIntervalSec: Float,
    val hpMultiplierPercent: Int = 100
)

class WaveManager {
    private val scripted = listOf(
        WaveConfig(enemyCount = 8, spawnIntervalSec = 0.9f, hpMultiplierPercent = 100),
        WaveConfig(enemyCount = 12, spawnIntervalSec = 0.8f, hpMultiplierPercent = 115),
        WaveConfig(enemyCount = 16, spawnIntervalSec = 0.7f, hpMultiplierPercent = 130),
        WaveConfig(enemyCount = 20, spawnIntervalSec = 0.65f, hpMultiplierPercent = 150)
    )

    fun totalScriptedWaves(): Int = scripted.size

    fun waveConfig(index: Int, difficulty: Difficulty): WaveConfig {
        return if (difficulty == Difficulty.ENDLESS) {
            val n = (index + 1).coerceAtLeast(1)
            val count = 8 + (n * 2)
            val interval = (0.95f - (n * 0.015f)).coerceAtLeast(0.35f)
            val mult = 100 + (n * 12)
            WaveConfig(enemyCount = count, spawnIntervalSec = interval, hpMultiplierPercent = mult)
        } else {
            val base = scripted[index.coerceIn(0, scripted.lastIndex)]
            val diffMul = when (difficulty) {
                Difficulty.EASY -> 80
                Difficulty.MEDIUM -> 100
                Difficulty.HARD -> 135
                Difficulty.ENDLESS -> 100
            }
            base.copy(hpMultiplierPercent = (base.hpMultiplierPercent * diffMul) / 100)
        }
    }
}
