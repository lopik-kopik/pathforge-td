package com.pathforge.core

import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.GameState
import com.pathforge.core.simulation.WaveManager
import org.junit.Assert.assertTrue
import org.junit.Test

class WaveSpawningTest {
    @Test
    fun endlessWavesIncreaseDifficulty() {
        val wm = WaveManager()
        val w1 = wm.waveConfig(index = 0, difficulty = Difficulty.ENDLESS)
        val w5 = wm.waveConfig(index = 4, difficulty = Difficulty.ENDLESS)

        assertTrue(w5.enemyCount > w1.enemyCount)
        assertTrue(w5.hpMultiplierPercent > w1.hpMultiplierPercent)
    }

    @Test
    fun scriptedWaveCountIsFinite() {
        val wm = WaveManager()
        assertTrue(wm.totalScriptedWaves() >= 4)
    }
}
