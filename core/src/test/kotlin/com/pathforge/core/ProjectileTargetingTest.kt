package com.pathforge.core

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.TowerType
import com.pathforge.core.simulation.GameWorld
import org.junit.Assert.assertTrue
import org.junit.Test

class ProjectileTargetingTest {
    @Test
    fun projectilesUseEntityIdAndDontCrashAfterEnemyRemoval() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.PlaceTower(2, 8, TowerType.SHERIFF))
        world.dispatch(GameAction.NextWave)

        repeat(1200) { world.update(1f / 60f) }

        assertTrue(world.projectiles.none { p -> world.enemies.none { it.id == p.targetId } })
    }
}
