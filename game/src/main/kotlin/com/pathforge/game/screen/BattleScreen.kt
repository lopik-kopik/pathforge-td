package com.pathforge.game.screen

import com.badlogic.gdx.Gdx
import com.badlogic.gdx.InputAdapter
import com.badlogic.gdx.ScreenAdapter
import com.badlogic.gdx.graphics.Color
import com.badlogic.gdx.graphics.GL20
import com.badlogic.gdx.graphics.OrthographicCamera
import com.badlogic.gdx.graphics.Texture
import com.badlogic.gdx.graphics.glutils.ShapeRenderer
import com.badlogic.gdx.math.Vector3
import com.badlogic.gdx.graphics.g2d.TextureRegion
import com.badlogic.gdx.utils.viewport.FitViewport
import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.CellType
import com.pathforge.core.domain.GameState
import com.pathforge.core.domain.TowerType
import com.pathforge.game.PathforgeGame

class BattleScreen(private val app: PathforgeGame) : ScreenAdapter() {
    private val world = app.world
    private val camera = OrthographicCamera()
    private val viewport = FitViewport(10f, 16f, camera)
    private val shape = ShapeRenderer()

    private var texGrass: Texture? = null
    private var texPath: Texture? = null
    private var texBlocked: Texture? = null
    private var texArcher: Texture? = null
    private var texSheriff: Texture? = null
    private var texMage: Texture? = null
    private var texSlime: Texture? = null
    private var texGoblin: Texture? = null
    private var texProjectile: Texture? = null

    private var regionArcher: TextureRegion? = null
    private var regionSheriff: TextureRegion? = null
    private var regionMage: TextureRegion? = null
    private var regionSlime: TextureRegion? = null
    private var regionGoblin: TextureRegion? = null
    private var regionProjectile: TextureRegion? = null

    private var animArcher: SpriteAnim? = null
    private var animSheriff: SpriteAnim? = null
    private var animMage: SpriteAnim? = null
    private var animSlime: SpriteAnim? = null
    private var animGoblin: SpriteAnim? = null
    private var animProjectile: SpriteAnim? = null

    private var animTime = 0f

    private var accumulator = 0f
    private val fixedStep = 1f / 60f

    override fun show() {
        viewport.apply()
        camera.position.set(5f, 8f, 0f)
        camera.update()

        texGrass = loadTextureOrNull("tile_grass.png")
        texPath = loadTextureOrNull("tile_path.png")
        texBlocked = loadTextureOrNull("tile_tree.png")
        texArcher = loadTextureOrNull("tower_archer.png")
        texSheriff = loadTextureOrNull("enemy_sheriff.png")
        texMage = loadTextureOrNull("wizards_team_square.png")
        if (texMage == null) {
            texMage = loadTextureOrNull("fire_wizard_attack.png")
        }
        texSlime = loadTextureOrNull("enemy_slime.png")
        texGoblin = loadTextureOrNull("enemy_goblin.png")
        texProjectile = loadTextureOrNull("projectile_arrow.png")

        animArcher = animForSprite(texArcher, 8f)
        animSheriff = animForSprite(texSheriff, 6f)
        animMage = animForSprite(texMage, 6f)
        animSlime = animForSprite(texSlime, 8f)
        animGoblin = animForSprite(texGoblin, 8f)
        animProjectile = animForSprite(texProjectile, 12f)

        regionArcher = animArcher?.frames?.firstOrNull()
        regionSheriff = animSheriff?.frames?.firstOrNull()
        regionMage = animMage?.frames?.firstOrNull() ?: regionCenterSquare(texMage)
        regionSlime = animSlime?.frames?.firstOrNull()
        regionGoblin = animGoblin?.frames?.firstOrNull()
        regionProjectile = animProjectile?.frames?.firstOrNull()

        Gdx.input.inputProcessor = object : InputAdapter() {
            override fun touchUp(screenX: Int, screenY: Int, pointer: Int, button: Int): Boolean {
                val v = viewport.unproject(Vector3(screenX.toFloat(), screenY.toFloat(), 0f))
                val gx = v.x.toInt()
                val gy = v.y.toInt()

                if (v.x in 8.5f..9.8f && v.y in 14.8f..15.8f) {
                    world.dispatch(GameAction.PauseToggle)
                    return true
                }

                if (v.x in 0.2f..2.0f && v.y in 0.2f..1.2f) {
                    world.dispatch(GameAction.SelectTowerType(TowerType.ARCHER))
                    return true
                }
                if (v.x in 2.2f..4.0f && v.y in 0.2f..1.2f) {
                    world.dispatch(GameAction.SelectTowerType(TowerType.SHERIFF))
                    return true
                }
                if (v.x in 4.2f..6.0f && v.y in 0.2f..1.2f) {
                    world.dispatch(GameAction.SelectTowerType(TowerType.MAGE))
                    return true
                }

                if (world.selectedTower != null) {
                    if (v.x in 3.2f..6.8f && v.y in 6.2f..7.3f) {
                        world.dispatch(GameAction.UpgradeSelectedTower)
                        return true
                    }
                    if (v.x in 3.2f..6.8f && v.y in 5.0f..6.0f) {
                        world.dispatch(GameAction.CloseUpgradeMenu)
                        return true
                    }
                }

                if (world.state == GameState.PAUSED) {
                    if (v.x in 3.0f..7.0f && v.y in 8.6f..9.6f) {
                        world.dispatch(GameAction.Resume)
                        return true
                    }
                    if (v.x in 3.0f..7.0f && v.y in 7.1f..8.1f) {
                        world.dispatch(GameAction.Restart)
                        return true
                    }
                    if (v.x in 3.0f..7.0f && v.y in 5.6f..6.6f) {
                        world.dispatch(GameAction.OpenMenu)
                        app.setScreen(MenuScreen(app))
                        return true
                    }
                }

                if (world.towers.any { it.gridX == gx && it.gridY == gy }) {
                    world.dispatch(GameAction.SelectTowerAt(gx, gy))
                } else {
                    world.dispatch(GameAction.CloseUpgradeMenu)
                    world.dispatch(GameAction.PlaceTower(gx, gy, world.selectedTowerType))
                }
                return true
            }
        }
    }

    override fun render(delta: Float) {
        animTime += delta.coerceAtMost(0.1f)
        accumulator += delta.coerceAtMost(0.1f)
        while (accumulator >= fixedStep) {
            world.update(fixedStep)
            accumulator -= fixedStep
        }

        if (world.state == GameState.WAVE_COMPLETE) {
            world.dispatch(GameAction.NextWave)
        }
        if (world.state == GameState.GAME_OVER || world.state == GameState.VICTORY) {
            app.setScreen(MenuScreen(app))
            return
        }

        Gdx.gl.glClearColor(0.12f, 0.2f, 0.1f, 1f)
        Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT)

        shape.projectionMatrix = camera.combined
        shape.begin(ShapeRenderer.ShapeType.Filled)

        for (x in 0 until 10) {
            for (y in 0 until 16) {
                when (world.map.getCell(x, y)) {
                    CellType.GRASS -> shape.color = Color(0.21f, 0.34f, 0.2f, 1f)
                    CellType.PATH -> shape.color = Color(0.5f, 0.42f, 0.28f, 1f)
                    CellType.BLOCKED -> shape.color = Color(0.2f, 0.25f, 0.2f, 1f)
                    CellType.TOWER -> shape.color = Color(0.21f, 0.34f, 0.2f, 1f)
                }
                shape.rect(x.toFloat(), y.toFloat(), 1f, 1f)
            }
        }

        shape.color = Color(0.1f, 0.1f, 0.1f, 0.84f)
        shape.rect(0f, 0f, 10f, 1.6f)

        shape.color = Color(0.16f, 0.35f, 0.18f, 1f)
        shape.rect(0.2f, 0.2f, 1.8f, 1f)
        shape.rect(2.2f, 0.2f, 1.8f, 1f)
        shape.rect(4.2f, 0.2f, 1.8f, 1f)

        val selectedX = when (world.selectedTowerType) {
            TowerType.ARCHER -> 0.2f
            TowerType.SHERIFF -> 2.2f
            TowerType.MAGE -> 4.2f
        }
        shape.color = Color.GOLD
        shape.rect(selectedX, 0.2f, 1.8f, 0.08f)

        shape.color = Color(0.15f, 0.15f, 0.15f, 0.95f)
        shape.rect(8.5f, 14.8f, 1.3f, 1f)

        if (world.state == GameState.PAUSED) {
            shape.color = Color(0f, 0f, 0f, 0.6f)
            shape.rect(0f, 0f, 10f, 16f)
            shape.color = Color(0.2f, 0.2f, 0.2f, 1f)
            shape.rect(3f, 8.6f, 4f, 1f)
            shape.rect(3f, 7.1f, 4f, 1f)
            shape.rect(3f, 5.6f, 4f, 1f)
        }

        if (world.selectedTower != null && world.state != GameState.PAUSED) {
            shape.color = Color(0f, 0f, 0f, 0.72f)
            shape.rect(2.8f, 4.6f, 4.4f, 3.1f)
            shape.color = if (world.canUpgradeSelectedTower) Color(0.2f, 0.45f, 0.22f, 1f) else Color(0.35f, 0.25f, 0.25f, 1f)
            shape.rect(3.2f, 6.2f, 3.6f, 1.1f)
            shape.color = Color(0.25f, 0.25f, 0.25f, 1f)
            shape.rect(3.2f, 5.0f, 3.6f, 1.0f)
        }

        shape.end()

        app.batch.projectionMatrix = camera.combined
        app.batch.begin()

        drawMapTextures()
        drawEntityTextures()

        app.font.draw(app.batch, "Gold: ${world.gold}", 0.2f, 15.55f)
        app.font.draw(app.batch, "HP: ${world.baseHp}", 3.6f, 15.55f)
        app.font.draw(app.batch, "Wave: ${world.currentWaveIndex + 1}", 6.2f, 15.55f)
        app.font.draw(app.batch, "A", 1.0f, 0.9f)
        app.font.draw(app.batch, "S", 3.0f, 0.9f)
        app.font.draw(app.batch, "M", 5.0f, 0.9f)
        app.font.draw(app.batch, "||", 9.4f, 15.5f)

        if (world.state == GameState.PAUSED) {
            app.font.draw(app.batch, "RESUME", 4.0f, 9.25f)
            app.font.draw(app.batch, "RESTART", 3.9f, 7.75f)
            app.font.draw(app.batch, "MENU", 4.25f, 6.25f)
        }

        val selected = world.selectedTower
        if (selected != null && world.state != GameState.PAUSED) {
            app.font.draw(app.batch, "Tower Lvl: ${selected.level}", 3.2f, 7.55f)
            app.font.draw(app.batch, "Dmg ${selected.damage}  Rng ${"%.1f".format(selected.range)}", 3.2f, 6.05f)
            val costText = if (selected.level >= 5) "MAX" else "Cost: ${world.selectedTowerUpgradeCost}"
            app.font.draw(app.batch, "UPGRADE $costText", 3.35f, 6.9f)
            app.font.draw(app.batch, "CLOSE", 4.35f, 5.65f)
        }

        app.batch.end()
    }

    private fun drawMapTextures() {
        for (x in 0 until 10) {
            for (y in 0 until 16) {
                when (world.map.getCell(x, y)) {
                    CellType.GRASS, CellType.TOWER -> drawCell(texGrass, x, y)
                    CellType.PATH -> drawCell(texPath, x, y)
                    CellType.BLOCKED -> drawCell(texBlocked, x, y)
                }
            }
        }
    }

    private fun drawEntityTextures() {
        for (tower in world.towers) {
            val tx = when (tower.type) {
                TowerType.ARCHER -> animArcher?.frame(animTime, tower.id.toFloat()) ?: regionArcher
                TowerType.SHERIFF -> animSheriff?.frame(animTime, tower.id.toFloat()) ?: regionSheriff
                TowerType.MAGE -> animMage?.frame(animTime, tower.id.toFloat()) ?: regionMage
            }
            tx?.let { app.batch.draw(it, tower.gridX + 0.025f, tower.gridY + 0.025f, 0.95f, 0.95f) }
            if (world.selectedTower?.id == tower.id) {
                app.font.draw(app.batch, "[${tower.level}]", tower.gridX + 0.22f, tower.gridY + 0.95f)
            }
        }

        for (enemy in world.enemies) {
            val region = when (enemy.type) {
                com.pathforge.core.domain.EnemyType.SLIME -> animSlime?.frame(animTime, enemy.id.toFloat()) ?: regionSlime
                com.pathforge.core.domain.EnemyType.GOBLIN -> animGoblin?.frame(animTime, enemy.id.toFloat()) ?: regionGoblin
                com.pathforge.core.domain.EnemyType.BAT -> animGoblin?.frame(animTime, enemy.id.toFloat()) ?: regionGoblin
                com.pathforge.core.domain.EnemyType.BOSS -> animGoblin?.frame(animTime, enemy.id.toFloat()) ?: regionGoblin
            }
            region?.let { app.batch.draw(it, enemy.x - 0.425f, enemy.y - 0.425f, 0.85f, 0.85f) }
        }

        for (p in world.projectiles) {
            val region = animProjectile?.frame(animTime, p.id.toFloat()) ?: regionProjectile
            region?.let { app.batch.draw(it, p.x - 0.11f, p.y - 0.11f, 0.22f, 0.22f) }
        }
    }

    private fun drawCell(texture: Texture?, x: Int, y: Int) {
        texture?.let { app.batch.draw(it, x.toFloat(), y.toFloat(), 1f, 1f) }
    }

    private fun loadTextureOrNull(path: String): Texture? {
        return try {
            Texture(Gdx.files.internal(path))
        } catch (_: Exception) {
            null
        }
    }

    private fun animForSprite(texture: Texture?, fps: Float): SpriteAnim? {
        if (texture == null) return null
        val w = texture.width
        val h = texture.height
        if (w > h && w % h == 0) {
            val frames = w / h
            val regions = TextureRegion.split(texture, h, h)
            if (frames > 0 && regions.isNotEmpty() && regions[0].isNotEmpty()) {
                val list = ArrayList<TextureRegion>(frames)
                for (i in 0 until frames) {
                    if (i < regions[0].size) {
                        list.add(regions[0][i])
                    }
                }
                if (list.isNotEmpty()) {
                    return SpriteAnim(list, fps)
                }
            }
        }
        return SpriteAnim(listOf(TextureRegion(texture)), fps)
    }

    private fun regionCenterSquare(texture: Texture?): TextureRegion? {
        if (texture == null) return null
        val w = texture.width
        val h = texture.height
        if (w == h) return TextureRegion(texture)
        val size = minOf(w, h)
        val x = (w - size) / 2
        val y = (h - size) / 2
        return TextureRegion(texture, x, y, size, size)
    }

    private data class SpriteAnim(val frames: List<TextureRegion>, val fps: Float) {
        fun frame(time: Float, seed: Float): TextureRegion {
            if (frames.size == 1) return frames[0]
            val offset = (seed * 0.13f) % 1f
            val idx = ((time + offset) * fps).toInt().coerceAtLeast(0) % frames.size
            return frames[idx]
        }
    }

    override fun resize(width: Int, height: Int) {
        viewport.update(width, height, true)
    }

    override fun dispose() {
        shape.dispose()
        texGrass?.dispose()
        texPath?.dispose()
        texBlocked?.dispose()
        texArcher?.dispose()
        texSheriff?.dispose()
        texMage?.dispose()
        texSlime?.dispose()
        texGoblin?.dispose()
        texProjectile?.dispose()
    }
}
