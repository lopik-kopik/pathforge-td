package com.pathforge.core

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.GameState
import com.pathforge.core.domain.TowerType
import com.pathforge.core.simulation.GameWorld
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class WaveAndCombatFlowTest {
    @Test
    fun waveSpawnsEnemiesAndTowersCanKill() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.PlaceTower(3, 9, TowerType.SHERIFF))
        world.dispatch(GameAction.NextWave)

        repeat(900) { world.update(1f / 60f) }

        assertTrue(world.waveProgress().second > 0)
        assertTrue(world.gold >= 0)
    }

    @Test
    fun baseLosesHpIfNoDefense() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.NextWave)
        val hpBefore = world.baseHp

        repeat(2200) { world.update(1f / 60f) }

        assertTrue(world.baseHp < hpBefore)
    }

    @Test
    fun gameOverTriggersWhenBaseZero() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.HARD))
        world.dispatch(GameAction.NextWave)

        repeat(15000) {
            world.update(1f / 60f)
            if (world.state == GameState.GAME_OVER) return@repeat
            if (world.state == GameState.WAVE_COMPLETE) world.dispatch(GameAction.NextWave)
        }

        assertEquals(GameState.GAME_OVER, world.state)
    }
}
