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

    override fun show() {
        viewport.apply()
        camera.position.set(5f, 8f, 0f)
        camera.update()

        bg = try {
            Texture(Gdx.files.internal("Gemini_Generated_Image_jdgjfkjdgjfkjdgj-removebg-preview.png"))
        } catch (_: Exception) {
            null
        }

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
        bg?.let { app.batch.draw(it, 0f, 0f, 10f, 16f) }
        app.batch.end()

        shape.projectionMatrix = camera.combined
        shape.begin(ShapeRenderer.ShapeType.Filled)
        shape.color = Color(0.18f, 0.48f, 0.24f, 0.95f)
        shape.rect(2f, 9f, 6f, 1.4f)
        shape.color = Color(0.22f, 0.42f, 0.22f, 0.95f)
        shape.rect(2f, 7f, 6f, 1.4f)
        shape.color = Color(0.55f, 0.25f, 0.2f, 0.95f)
        shape.rect(2f, 5f, 6f, 1.4f)
        shape.color = Color(0.2f, 0.3f, 0.55f, 0.95f)
        shape.rect(2f, 3f, 6f, 1.4f)
        shape.end()

        app.batch.projectionMatrix = camera.combined
        app.batch.begin()
        app.font.draw(app.batch, "Pathforge TD - libGDX", 2f, 13f)
        app.font.draw(app.batch, "Menu Coins: ${world.menuCoins}", 2f, 12f)
        app.font.draw(app.batch, "Start Easy", 4f, 9.9f)
        app.font.draw(app.batch, "Start Medium", 3.7f, 7.9f)
        app.font.draw(app.batch, "Start Hard", 3.9f, 5.9f)
        app.font.draw(app.batch, "Sandbox", 4.2f, 3.9f)
        app.font.draw(app.batch, "Cloud sync disabled (temporary)", 2.2f, 1.2f)
        app.batch.end()
    }

    override fun resize(width: Int, height: Int) {
        viewport.update(width, height, true)
    }

    override fun dispose() {
        shape.dispose()
        bg?.dispose()
    }
}
