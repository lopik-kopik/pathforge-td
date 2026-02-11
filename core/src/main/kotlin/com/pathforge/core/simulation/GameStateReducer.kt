package com.pathforge.core.simulation

import com.pathforge.core.domain.GameState

object GameStateReducer {
    fun reduce(current: GameState, event: TransitionEvent): GameState {
        return when (event) {
            TransitionEvent.OPEN_MENU -> GameState.MENU
            TransitionEvent.OPEN_DIFFICULTY -> if (current == GameState.MENU) GameState.DIFFICULTY_SELECT else current
            TransitionEvent.START_PLAY -> GameState.WAVE_COMPLETE
            TransitionEvent.PAUSE -> if (current == GameState.PLAYING) GameState.PAUSED else current
            TransitionEvent.RESUME -> if (current == GameState.PAUSED) GameState.PLAYING else current
            TransitionEvent.GAME_OVER -> GameState.GAME_OVER
            TransitionEvent.VICTORY -> GameState.VICTORY
            TransitionEvent.ENTER_SANDBOX -> GameState.SANDBOX
            TransitionEvent.EXIT_SANDBOX -> GameState.MENU
        }
    }
}

enum class TransitionEvent {
    OPEN_MENU,
    OPEN_DIFFICULTY,
    START_PLAY,
    PAUSE,
    RESUME,
    GAME_OVER,
    VICTORY,
    ENTER_SANDBOX,
    EXIT_SANDBOX
}
