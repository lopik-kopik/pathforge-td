package com.pathforge.core.simulation

import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.Enemy
import com.pathforge.core.domain.EnemyType
import com.pathforge.core.domain.GameMap
import com.pathforge.core.domain.GameState
import com.pathforge.core.domain.ProgressData
import com.pathforge.core.domain.Projectile
import com.pathforge.core.domain.Tower
import com.pathforge.core.domain.TowerType
import com.pathforge.core.domain.towerMaxLevel
import com.pathforge.core.domain.towerUpgradeCost
import com.pathforge.core.persistence.ProgressRepository
import kotlin.math.hypot
import kotlin.math.max

class GameWorld(private val progressRepository: ProgressRepository? = null) {
    companion object {
        const val MAP_COLS = GameMap.COLS
        const val MAP_ROWS = GameMap.ROWS
        private const val START_WAVE_DELAY = 3f
    }

    val map = GameMap()

    var state: GameState = GameState.LOGIN
        private set

    var difficulty: Difficulty = Difficulty.MEDIUM
        private set

    var selectedTowerType: TowerType = TowerType.ARCHER
        private set

    var gold: Int = 50
        private set

    var baseHp: Int = 20
        private set

    var currentWaveIndex: Int = 0
        private set

    var endlessWaveNumber: Int = 1
        private set

    var menuCoins: Int = 0
        private set

    var cards: Int = 0
        private set

    val selectedTower: Tower?
        get() = selectedTowerId?.let { id -> towerList.firstOrNull { it.id == id } }

    val selectedTowerUpgradeCost: Int
        get() = selectedTower?.let { towerUpgradeCost(it.level) } ?: 0

    val canUpgradeSelectedTower: Boolean
        get() {
            val t = selectedTower ?: return false
            if (t.level >= towerMaxLevel()) return false
            if (state == GameState.SANDBOX) return true
            return gold >= towerUpgradeCost(t.level)
        }

    val towers: List<Tower> get() = towerList
    val enemies: List<Enemy> get() = enemyList
    val projectiles: List<Projectile> get() = projectileList

    private val towerList = mutableListOf<Tower>()
    private val enemyList = mutableListOf<Enemy>()
    private val projectileList = mutableListOf<Projectile>()

    private var selectedTowerId: Long? = null

    private var nextEntityId = 1L
    private var waveTimer = START_WAVE_DELAY
    private var spawnTimer = 0f
    private var spawnedInWave = 0
    private var totalInWave = 0
    private var waveActive = false

    private val waveManager = WaveManager()

    fun bootstrap() {
        val progress = progressRepository?.load() ?: ProgressData()
        menuCoins = progress.menuCoins
        cards = progress.cards
        state = GameState.MENU
    }

    fun dispatch(action: GameAction) {
        when (action) {
            GameAction.OpenMenu -> {
                selectedTowerId = null
                state = GameStateReducer.reduce(state, TransitionEvent.OPEN_MENU)
            }
            GameAction.OpenDifficultySelect -> state = GameStateReducer.reduce(state, TransitionEvent.OPEN_DIFFICULTY)
            is GameAction.StartGame -> startGame(action.difficulty)
            GameAction.PauseToggle -> {
                state = if (state == GameState.PAUSED) {
                    GameStateReducer.reduce(state, TransitionEvent.RESUME)
                } else {
                    GameStateReducer.reduce(state, TransitionEvent.PAUSE)
                }
            }
            GameAction.Resume -> state = GameStateReducer.reduce(state, TransitionEvent.RESUME)
            is GameAction.PlaceTower -> placeTower(action.gridX, action.gridY, action.type)
            is GameAction.SelectTowerType -> selectedTowerType = action.type
            is GameAction.SelectTowerAt -> selectTowerAt(action.gridX, action.gridY)
            GameAction.CloseUpgradeMenu -> selectedTowerId = null
            GameAction.UpgradeSelectedTower -> upgradeSelectedTower()
            GameAction.NextWave -> if (state == GameState.WAVE_COMPLETE || state == GameState.SANDBOX) startNextWave()
            GameAction.EnterSandbox -> enterSandbox()
            GameAction.ExitSandbox -> {
                selectedTowerId = null
                state = GameStateReducer.reduce(state, TransitionEvent.EXIT_SANDBOX)
            }
            GameAction.Restart -> startGame(difficulty)
            is GameAction.ForceState -> state = action.state
        }
    }

    fun update(dt: Float) {
        if (state == GameState.PAUSED || state == GameState.MENU || state == GameState.LOGIN || state == GameState.DIFFICULTY_SELECT || state == GameState.CHARACTER_MENU || state == GameState.GAME_OVER || state == GameState.VICTORY) {
            return
        }

        if (state == GameState.WAVE_COMPLETE) {
            waveTimer -= dt
            if (waveTimer <= 0f) {
                startNextWave()
            }
            return
        }

        if (waveActive) {
            updateSpawning(dt)
        }
        updateTowers(dt)
        updateProjectiles(dt)
        updateEnemies(dt)
        cleanupDead()

        if (baseHp <= 0) {
            baseHp = 0
            state = GameStateReducer.reduce(state, TransitionEvent.GAME_OVER)
        }

        if (waveActive && spawnedInWave >= totalInWave && enemyList.isEmpty()) {
            waveActive = false
            if (difficulty == Difficulty.ENDLESS || state == GameState.SANDBOX) {
                endlessWaveNumber += 1
                state = GameState.WAVE_COMPLETE
                waveTimer = 2f
            } else {
                currentWaveIndex += 1
                if (currentWaveIndex >= waveManager.totalScriptedWaves()) {
                    menuCoins += when (difficulty) {
                        Difficulty.EASY -> 50
                        Difficulty.MEDIUM -> 100
                        Difficulty.HARD -> 250
                        Difficulty.ENDLESS -> 0
                    }
                    persistProgress()
                    state = GameStateReducer.reduce(state, TransitionEvent.VICTORY)
                } else {
                    state = GameState.WAVE_COMPLETE
                    waveTimer = 3f
                }
            }
        }
    }

    fun waveProgress(): Pair<Int, Int> = spawnedInWave to totalInWave

    private fun startGame(newDifficulty: Difficulty) {
        difficulty = newDifficulty
        currentWaveIndex = 0
        endlessWaveNumber = 1
        waveActive = false
        waveTimer = START_WAVE_DELAY
        state = GameState.WAVE_COMPLETE
        selectedTowerType = TowerType.ARCHER
        selectedTowerId = null

        when (difficulty) {
            Difficulty.EASY -> {
                gold = 75
                baseHp = 25
            }
            Difficulty.MEDIUM -> {
                gold = 50
                baseHp = 20
            }
            Difficulty.HARD -> {
                gold = 35
                baseHp = 15
            }
            Difficulty.ENDLESS -> {
                gold = 50
                baseHp = 15
            }
        }

        towerList.clear()
        enemyList.clear()
        projectileList.clear()
    }

    private fun enterSandbox() {
        state = GameState.SANDBOX
        gold = 9999
        baseHp = 999
        waveActive = false
        enemyList.clear()
        projectileList.clear()
        towerList.clear()
        endlessWaveNumber = 1
        selectedTowerId = null
    }

    private fun startNextWave() {
        state = GameState.PLAYING
        waveActive = true
        spawnedInWave = 0
        spawnTimer = 0f

        val config = waveManager.waveConfig(waveIndexForCurrentMode(), difficulty)
        totalInWave = config.enemyCount
    }

    private fun waveIndexForCurrentMode(): Int {
        return if (difficulty == Difficulty.ENDLESS || state == GameState.SANDBOX) {
            max(0, endlessWaveNumber - 1)
        } else {
            currentWaveIndex
        }
    }

    private fun updateSpawning(dt: Float) {
        if (spawnedInWave >= totalInWave) return

        val config = waveManager.waveConfig(waveIndexForCurrentMode(), if (state == GameState.SANDBOX) Difficulty.ENDLESS else difficulty)
        spawnTimer -= dt
        if (spawnTimer <= 0f) {
            spawnEnemy(config.hpMultiplierPercent)
            spawnedInWave += 1
            spawnTimer = config.spawnIntervalSec
        }
    }

    private fun spawnEnemy(hpMultiplierPercent: Int) {
        val id = nextId()
        val hp = (100 * hpMultiplierPercent) / 100
        val start = map.waypoints.first()
        enemyList += Enemy(
            id = id,
            type = chooseEnemyType(),
            x = start.x,
            y = start.y,
            hp = hp,
            speed = 1.0f + (hpMultiplierPercent - 100) * 0.002f,
            reward = 5,
            waypointIndex = 1
        )
    }

    private fun chooseEnemyType(): EnemyType {
        val idx = spawnedInWave
        return when {
            idx % 11 == 0 && idx > 0 -> EnemyType.BOSS
            idx % 5 == 0 -> EnemyType.GOBLIN
            idx % 7 == 0 -> EnemyType.BAT
            else -> EnemyType.SLIME
        }
    }

    private fun updateTowers(dt: Float) {
        for (i in towerList.indices) {
            val tower = towerList[i]
            val cd = (tower.cooldown - dt).coerceAtLeast(0f)
            towerList[i] = tower.copy(cooldown = cd)

            if (cd > 0f) continue

            val target = enemyList.firstOrNull { enemy ->
                enemy.hp > 0 && !enemy.reachedEnd && withinRange(tower, enemy)
            } ?: continue

            projectileList += Projectile(
                id = nextId(),
                x = tower.gridX + 0.5f,
                y = tower.gridY + 0.5f,
                targetId = target.id,
                damage = tower.damage
            )
            towerList[i] = tower.copy(cooldown = tower.fireRate)
        }
    }

    private fun withinRange(tower: Tower, enemy: Enemy): Boolean {
        val dx = tower.gridX + 0.5f - enemy.x
        val dy = tower.gridY + 0.5f - enemy.y
        return hypot(dx, dy) <= tower.range
    }

    private fun updateProjectiles(dt: Float) {
        val it = projectileList.listIterator()
        while (it.hasNext()) {
            val p = it.next()
            val target = enemyList.firstOrNull { it.id == p.targetId && it.hp > 0 }
            if (target == null) {
                it.remove()
                continue
            }

            val dx = target.x - p.x
            val dy = target.y - p.y
            val dist = hypot(dx, dy)
            if (dist < 0.12f) {
                target.hp -= p.damage
                it.remove()
                continue
            }

            val nx = p.x + (dx / dist) * p.speed * dt
            val ny = p.y + (dy / dist) * p.speed * dt
            it.set(p.copy(x = nx, y = ny))
        }
    }

    private fun updateEnemies(dt: Float) {
        val waypoints = map.waypoints
        for (enemy in enemyList) {
            if (enemy.hp <= 0 || enemy.reachedEnd) continue

            if (enemy.waypointIndex >= waypoints.size) {
                enemy.reachedEnd = true
                baseHp -= 1
                continue
            }

            val target = waypoints[enemy.waypointIndex]
            val dx = target.x - enemy.x
            val dy = target.y - enemy.y
            val dist = hypot(dx, dy)

            if (dist < 0.08f) {
                enemy.waypointIndex += 1
                continue
            }

            enemy.x += (dx / dist) * enemy.speed * dt
            enemy.y += (dy / dist) * enemy.speed * dt
        }
    }

    private fun cleanupDead() {
        val deadRewards = enemyList.count { it.hp <= 0 && !it.reachedEnd }
        if (deadRewards > 0) {
            gold += deadRewards * 5
        }
        enemyList.removeAll { it.hp <= 0 || it.reachedEnd }
        projectileList.removeAll { p -> enemyList.none { e -> e.id == p.targetId } }
    }

    private fun placeTower(gridX: Int, gridY: Int, type: TowerType) {
        if (state != GameState.PLAYING && state != GameState.SANDBOX && state != GameState.WAVE_COMPLETE) {
            return
        }
        if (gridX !in 0 until MAP_COLS || gridY !in 0 until MAP_ROWS) return

        val existing = towerList.firstOrNull { it.gridX == gridX && it.gridY == gridY }
        if (existing != null) {
            selectedTowerId = existing.id
            return
        }

        if (!map.canPlaceTower(gridX, gridY)) return

        val cost = when (type) {
            TowerType.ARCHER -> 25
            TowerType.SHERIFF -> 45
            TowerType.MAGE -> 60
        }
        if (state != GameState.SANDBOX && gold < cost) return

        towerList += createTower(id = nextId(), gridX = gridX, gridY = gridY, type = type, level = 1)
        map.placeTower(gridX, gridY)
        selectedTowerId = null
        if (state != GameState.SANDBOX) gold -= cost
    }

    private fun selectTowerAt(gridX: Int, gridY: Int) {
        selectedTowerId = towerList.firstOrNull { it.gridX == gridX && it.gridY == gridY }?.id
    }

    private fun upgradeSelectedTower() {
        val selected = selectedTower ?: return
        if (selected.level >= towerMaxLevel()) return
        val cost = towerUpgradeCost(selected.level)
        if (state != GameState.SANDBOX && gold < cost) return

        val idx = towerList.indexOfFirst { it.id == selected.id }
        if (idx < 0) return

        val upgraded = createTower(
            id = selected.id,
            gridX = selected.gridX,
            gridY = selected.gridY,
            type = selected.type,
            level = selected.level + 1,
            cooldown = selected.cooldown
        )
        towerList[idx] = upgraded
        if (state != GameState.SANDBOX) gold -= cost
    }

    private fun createTower(
        id: Long,
        gridX: Int,
        gridY: Int,
        type: TowerType,
        level: Int,
        cooldown: Float = 0f
    ): Tower {
        val (baseDamage, baseRange, baseFireRate, dmgPerLevel, rangePerLevel, firePerLevel) = when (type) {
            TowerType.ARCHER -> Stats(12, 4.0f, 0.7f, 5, 0.3f, 0.2f)
            TowerType.SHERIFF -> Stats(22, 5.5f, 1.1f, 8, 0.25f, 0.15f)
            TowerType.MAGE -> Stats(18, 4.5f, 0.9f, 6, 0.3f, 0.18f)
        }
        val l = (level - 1).coerceAtLeast(0)

        return Tower(
            id = id,
            gridX = gridX,
            gridY = gridY,
            type = type,
            level = level,
            cooldown = cooldown,
            damage = baseDamage + l * dmgPerLevel,
            range = baseRange + l * rangePerLevel,
            fireRate = baseFireRate + l * firePerLevel
        )
    }

    fun saveProgress() {
        persistProgress()
    }

    private fun persistProgress() {
        progressRepository?.save(
            ProgressData(
                schemaVersion = 2,
                menuCoins = menuCoins,
                cards = cards,
                archerLevel = 1,
                sheriffLevel = 1,
                allyLevel = 1
            )
        )
    }

    private fun nextId(): Long {
        val id = nextEntityId
        nextEntityId += 1
        return id
    }

    private data class Stats(
        val baseDamage: Int,
        val baseRange: Float,
        val baseFireRate: Float,
        val dmgPerLevel: Int,
        val rangePerLevel: Float,
        val firePerLevel: Float
    )
}
