package com.pathforge.game.screen

import com.badlogic.gdx.Gdx
import com.badlogic.gdx.InputAdapter
import com.badlogic.gdx.ScreenAdapter
import com.badlogic.gdx.graphics.Color
import com.badlogic.gdx.graphics.GL20
import com.badlogic.gdx.graphics.OrthographicCamera
import com.badlogic.gdx.graphics.Texture
import com.badlogic.gdx.graphics.g2d.GlyphLayout
import com.badlogic.gdx.graphics.g2d.TextureRegion
import com.badlogic.gdx.graphics.glutils.ShapeRenderer
import com.badlogic.gdx.math.Vector3
import com.badlogic.gdx.utils.TimeUtils
import com.badlogic.gdx.utils.viewport.FitViewport
import com.pathforge.core.action.GameAction
import com.pathforge.core.domain.Difficulty
import com.pathforge.core.domain.GameState
import com.pathforge.game.PathforgeGame

class MenuScreen(private val app: PathforgeGame) : ScreenAdapter() {
    private val world = app.world
    private val camera = OrthographicCamera()
    private val viewport = FitViewport(10f, 16f, camera)
    private val shape = ShapeRenderer()
    private var bg: Texture? = null
    private val layout = GlyphLayout()
    private var chestRegion: TextureRegion? = null
    private var lastChestClaimAtMs = 0L
    private val chestCooldownMs = 3000L

    override fun show() {
        viewport.apply()
        camera.position.set(5f, 8f, 0f)
        camera.update()

        bg = try {
            Texture(Gdx.files.internal("Gemini_Generated_Image_jdgjfkjdgjfkjdgj-removebg-preview.png"))
        } catch (_: Exception) {
            null
        }
        chestRegion = centerSquareRegion(bg)

        world.dispatch(GameAction.OpenMenu)

        Gdx.input.inputProcessor = object : InputAdapter() {
            override fun touchUp(screenX: Int, screenY: Int, pointer: Int, button: Int): Boolean {
                val v = viewport.unproject(Vector3(screenX.toFloat(), screenY.toFloat(), 0f))

                if (v.x in 2f..8f && v.y in 9f..10.4f) {
                    world.dispatch(GameAction.StartGame(Difficulty.EASY))
                    app.setScreen(BattleScreen(app))
                    return true
                }
                if (v.x in 2f..8f && v.y in 7f..8.4f) {
                    world.dispatch(GameAction.StartGame(Difficulty.MEDIUM))
                    app.setScreen(BattleScreen(app))
                    return true
                }
                if (v.x in 2f..8f && v.y in 5f..6.4f) {
                    world.dispatch(GameAction.StartGame(Difficulty.HARD))
                    app.setScreen(BattleScreen(app))
                    return true
                }
                if (v.x in 2f..8f && v.y in 3f..4.4f) {
                    world.dispatch(GameAction.EnterSandbox)
                    app.setScreen(BattleScreen(app))
                    return true
                }
                if (v.x in 8.2f..9.6f && v.y in 13.4f..14.8f) {
                    val now = TimeUtils.millis()
                    if (now - lastChestClaimAtMs >= chestCooldownMs) {
                        world.dispatch(GameAction.AddMenuCoins(5))
                        lastChestClaimAtMs = now
                    }
                    return true
                }
                return false
            }
        }
    }

    override fun render(delta: Float) {
        if (world.state == GameState.WAVE_COMPLETE || world.state == GameState.PLAYING || world.state == GameState.SANDBOX) {
            app.setScreen(BattleScreen(app))
            return
        }

        Gdx.gl.glClearColor(0.06f, 0.09f, 0.07f, 1f)
        Gdx.gl.glClear(GL20.GL_COLOR_BUFFER_BIT)

        app.batch.projectionMatrix = camera.combined
        app.batch.begin()
        chestRegion?.let { app.batch.draw(it, 8.2f, 13.4f, 1.4f, 1.4f) }
        app.batch.end()

        shape.projectionMatrix = camera.combined
        shape.begin(ShapeRenderer.ShapeType.Filled)
        drawPanel(2f, 9f, 6f, 1.4f, Color(0.18f, 0.48f, 0.24f, 0.95f))
        drawPanel(2f, 7f, 6f, 1.4f, Color(0.22f, 0.42f, 0.22f, 0.95f))
        drawPanel(2f, 5f, 6f, 1.4f, Color(0.55f, 0.25f, 0.2f, 0.95f))
        drawPanel(2f, 3f, 6f, 1.4f, Color(0.2f, 0.3f, 0.55f, 0.95f))
        shape.end()

        app.batch.projectionMatrix = camera.combined
        app.batch.begin()
        drawCentered("Pathforge TD", 0f, 12.7f, 10f)
        drawCentered("Menu Coins: ${world.menuCoins}", 0f, 11.8f, 10f)
        drawCentered("Start Easy", 2f, 9.95f, 6f)
        drawCentered("Start Medium", 2f, 7.95f, 6f)
        drawCentered("Start Hard", 2f, 5.95f, 6f)
        drawCentered("Sandbox", 2f, 3.95f, 6f)
        drawSmallCentered("Cloud sync disabled", 0f, 1.5f, 10f)
        drawSmallCentered("(temporary)", 0f, 0.9f, 10f)
        app.batch.end()
    }

    override fun resize(width: Int, height: Int) {
        viewport.update(width, height, true)
    }

    override fun dispose() {
        shape.dispose()
        bg?.dispose()
    }

    private fun drawCentered(text: String, x: Float, y: Float, width: Float) {
        layout.setText(app.font, text)
        app.font.draw(app.batch, text, x + (width - layout.width) * 0.5f, y)
    }

    private fun drawSmallCentered(text: String, x: Float, y: Float, width: Float) {
        val data = app.font.data
        val oldScaleX = data.scaleX
        val oldScaleY = data.scaleY
        data.setScale(oldScaleX * 0.85f, oldScaleY * 0.85f)
        layout.setText(app.font, text)
        app.font.draw(app.batch, text, x + (width - layout.width) * 0.5f, y)
        data.setScale(oldScaleX, oldScaleY)
    }

    private fun drawPanel(x: Float, y: Float, w: Float, h: Float, fill: Color) {
        val border = 0.06f
        shape.color = Color(0f, 0f, 0f, 0.45f)
        shape.rect(x - border, y - border, w + border * 2f, h + border * 2f)
        shape.color = fill
        shape.rect(x, y, w, h)
    }

    private fun centerSquareRegion(texture: Texture?): TextureRegion? {
        if (texture == null) return null
        val w = texture.width
        val h = texture.height
        if (w == h) return TextureRegion(texture)
        val size = minOf(w, h)
        val x = (w - size) / 2
        val y = (h - size) / 2
        return TextureRegion(texture, x, y, size, size)
    }
}
