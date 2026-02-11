package com.pathforge.game

import com.badlogic.gdx.Game
import com.badlogic.gdx.Gdx
import com.badlogic.gdx.graphics.Texture
import com.badlogic.gdx.graphics.g2d.BitmapFont
import com.badlogic.gdx.graphics.g2d.SpriteBatch
import com.badlogic.gdx.graphics.g2d.freetype.FreeTypeFontGenerator
import com.pathforge.core.persistence.ProgressRepository
import com.pathforge.core.simulation.GameWorld
import com.pathforge.game.screen.MenuScreen

class PathforgeGame(private val repository: ProgressRepository) : Game() {
    lateinit var world: GameWorld
        private set

    lateinit var font: BitmapFont
        private set

    lateinit var batch: SpriteBatch
        private set

    override fun create() {
        batch = SpriteBatch()
        font = createUiFont()
        world = GameWorld(repository)
        world.bootstrap()
        setScreen(MenuScreen(this))
    }

    override fun dispose() {
        world.saveProgress()
        screen?.dispose()
        font.dispose()
        batch.dispose()
    }

    private fun createUiFont(): BitmapFont {
        val preferred = "fonts/pixelify.ttf"
        val fallback = "fonts/ui_font.ttf"
        val fontFile = when {
            Gdx.files.internal(preferred).exists() -> Gdx.files.internal(preferred)
            Gdx.files.internal(fallback).exists() -> Gdx.files.internal(fallback)
            else -> null
        }

        val targetCapHeight = 0.45f
        if (fontFile == null) {
            return BitmapFont().apply {
                data.setScale(targetCapHeight / data.capHeight)
            }
        }

        val generator = FreeTypeFontGenerator(fontFile)
        val params = FreeTypeFontGenerator.FreeTypeFontParameter().apply {
            // Generate at a higher pixel size, then scale to world units for crisp text.
            size = (52 * Gdx.graphics.density).toInt().coerceAtLeast(26)
            color = com.badlogic.gdx.graphics.Color.WHITE
            borderWidth = 0.8f
            borderColor = com.badlogic.gdx.graphics.Color.BLACK
            minFilter = Texture.TextureFilter.Nearest
            magFilter = Texture.TextureFilter.Nearest
            characters = buildUiCharacters()
            incremental = false
        }

        val generated = generator.generateFont(params)
        generator.dispose()
        generated.data.setScale(targetCapHeight / generated.capHeight)
        generated.setUseIntegerPositions(false)
        return generated
    }

    private fun buildUiCharacters(): String {
        val sb = StringBuilder()
        for (code in 32..126) sb.append(code.toChar())
        for (code in 0x0400..0x04FF) sb.append(code.toChar())
        sb.append("€₽£¥•—–«»№…✓✕→←↑↓")
        return sb.toString()
    }
}
