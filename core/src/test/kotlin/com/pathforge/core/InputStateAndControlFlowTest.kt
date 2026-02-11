package com.pathforge.core

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.GameState
import com.pathforge.core.domain.TowerType
import com.pathforge.core.simulation.GameWorld
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class InputStateAndControlFlowTest {
    @Test
    fun pauseToggleRoundTripWorks() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.NextWave)

        world.dispatch(GameAction.PauseToggle)
        assertEquals(GameState.PAUSED, world.state)

        world.dispatch(GameAction.PauseToggle)
        assertEquals(GameState.PLAYING, world.state)
    }

    @Test
    fun selectingTowerTypePersistsChoice() {
        val world = GameWorld()
        world.dispatch(GameAction.SelectTowerType(TowerType.MAGE))
        assertEquals(TowerType.MAGE, world.selectedTowerType)
    }

    @Test
    fun restartResetsWaveAndEntities() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.PlaceTower(2, 2, TowerType.ARCHER))
        world.dispatch(GameAction.NextWave)
        repeat(300) { world.update(1f / 60f) }

        world.dispatch(GameAction.Restart)

        assertEquals(GameState.WAVE_COMPLETE, world.state)
        assertEquals(0, world.currentWaveIndex)
        assertTrue(world.towers.isEmpty())
        assertTrue(world.enemies.isEmpty())
        assertTrue(world.projectiles.isEmpty())
    }
}
