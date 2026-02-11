package com.pathforge.core

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.TowerType
import com.pathforge.core.simulation.GameWorld
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class EconomyTest {
    @Test
    fun placingTowerConsumesGold() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))

        val before = world.gold
        world.dispatch(GameAction.PlaceTower(2, 2, TowerType.ARCHER))
        val after = world.gold

        assertEquals(before - 25, after)
    }

    @Test
    fun deadEnemyAwardsGold() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.PlaceTower(2, 9, TowerType.SHERIFF))
        world.dispatch(GameAction.NextWave)

        val before = world.gold
        repeat(1200) { world.update(1f / 60f) }

        assertTrue(world.gold >= before)
    }
}
