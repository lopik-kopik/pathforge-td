package com.pathforge.core

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.CellType
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.GameState
import com.pathforge.core.domain.TowerType
import com.pathforge.core.simulation.GameWorld
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class PlacementAndMapRulesTest {
    @Test
    fun cannotPlaceTowerOnPathOrBlocked() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))

        val pathCell = 5 to 15
        val blockedCell = 0 to 15

        assertEquals(CellType.PATH, world.map.getCell(pathCell.first, pathCell.second))
        assertEquals(CellType.BLOCKED, world.map.getCell(blockedCell.first, blockedCell.second))

        val before = world.towers.size
        world.dispatch(GameAction.PlaceTower(pathCell.first, pathCell.second, TowerType.ARCHER))
        world.dispatch(GameAction.PlaceTower(blockedCell.first, blockedCell.second, TowerType.ARCHER))

        assertEquals(before, world.towers.size)
    }

    @Test
    fun canPlaceTowerOnGrassAndGoldDecreases() {
        val world = GameWorld()
        world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))

        val goldBefore = world.gold
        world.dispatch(GameAction.PlaceTower(2, 2, TowerType.ARCHER))

        assertEquals(1, world.towers.size)
        assertEquals(goldBefore - 25, world.gold)
        assertEquals(CellType.TOWER, world.map.getCell(2, 2))
    }

    @Test
    fun sandboxAllowsPlacingWithoutGoldLimit() {
        val world = GameWorld()
        world.dispatch(GameAction.EnterSandbox)
        val before = world.gold

        world.dispatch(GameAction.PlaceTower(2, 2, TowerType.MAGE))

        assertEquals(1, world.towers.size)
        assertEquals(before, world.gold)
    }
}
