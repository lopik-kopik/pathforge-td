package com.pathforge.core.action

import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.GameState
import com.pathforge.core.domain.TowerType

sealed interface GameAction {
    data object OpenMenu : GameAction
    data object OpenDifficultySelect : GameAction
    data class StartGame(val difficulty: Difficulty) : GameAction
    data object PauseToggle : GameAction
    data object Resume : GameAction
    data class PlaceTower(val gridX: Int, val gridY: Int, val type: TowerType) : GameAction
    data class SelectTowerType(val type: TowerType) : GameAction
    data class SelectTowerAt(val gridX: Int, val gridY: Int) : GameAction
    data object CloseUpgradeMenu : GameAction
    data object UpgradeSelectedTower : GameAction
    data object NextWave : GameAction
    data object EnterSandbox : GameAction
    data object ExitSandbox : GameAction
    data object Restart : GameAction
    data class AddMenuCoins(val amount: Int) : GameAction
    data class ForceState(val state: GameState) : GameAction
}
