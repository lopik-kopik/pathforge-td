package com.pathforge.core

import com.pathforge.core.domain.GameState
import com.pathforge.core.simulation.GameStateReducer
import com.pathforge.core.simulation.TransitionEvent
import org.junit.Assert.assertEquals
import org.junit.Test

class StateTransitionTest {
    @Test
    fun menuToDifficultyTransition() {
        val out = GameStateReducer.reduce(GameState.MENU, TransitionEvent.OPEN_DIFFICULTY)
        assertEquals(GameState.DIFFICULTY_SELECT, out)
    }

    @Test
    fun pauseOnlyFromPlaying() {
        val fromMenu = GameStateReducer.reduce(GameState.MENU, TransitionEvent.PAUSE)
        val fromPlaying = GameStateReducer.reduce(GameState.PLAYING, TransitionEvent.PAUSE)
        assertEquals(GameState.MENU, fromMenu)
        assertEquals(GameState.PAUSED, fromPlaying)
    }
}
