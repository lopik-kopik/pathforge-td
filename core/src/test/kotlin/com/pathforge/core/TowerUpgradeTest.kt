package com.pathforge.core

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.TowerType
import com.pathforge.core.simulation.GameWorld
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class TowerUpgradeTest {
    @Test
    fun selectingTowerAndUpgradingIncreasesLevelAndStats() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.EASY))
        world.dispatch(GameAction.PlaceTower(2, 2, TowerType.ARCHER))

        val beforeGold = world.gold
        world.dispatch(GameAction.SelectTowerAt(2, 2))
        val before = world.selectedTower ?: error("tower should be selected")

        world.dispatch(GameAction.UpgradeSelectedTower)
        val after = world.selectedTower ?: error("tower should stay selected")

        assertEquals(2, after.level)
        assertTrue(after.damage > before.damage)
        assertTrue(after.range > before.range)
        assertTrue(after.fireRate > before.fireRate)
        assertEquals(beforeGold - 30, world.gold)
    }

    @Test
    fun cannotUpgradeWithoutGold() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
        world.dispatch(GameAction.PlaceTower(2, 2, TowerType.SHERIFF))
        world.dispatch(GameAction.SelectTowerAt(2, 2))
        val t = world.selectedTower ?: error("tower must exist")

        val levelBefore = t.level
        world.dispatch(GameAction.UpgradeSelectedTower)
        val levelAfter = world.selectedTower?.level ?: levelBefore

        assertTrue(world.gold < 30)
        assertEquals(levelBefore, levelAfter)
    }

    @Test
    fun upgradeCostProgressionMatchesLegacy() {
        assertEquals(30, com.pathforge.core.domain.towerUpgradeCost(1))
        assertEquals(50, com.pathforge.core.domain.towerUpgradeCost(2))
        assertEquals(100, com.pathforge.core.domain.towerUpgradeCost(3))
        assertEquals(200, com.pathforge.core.domain.towerUpgradeCost(4))
        assertEquals(0, com.pathforge.core.domain.towerUpgradeCost(5))
    }
}
