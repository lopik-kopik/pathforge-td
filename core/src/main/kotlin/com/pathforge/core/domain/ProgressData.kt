package com.pathforge.core.domain

data class ProgressData(
    val schemaVersion: Int = 2,
    val menuCoins: Int = 0,
    val cards: Int = 0,
    val archerLevel: Int = 1,
    val sheriffLevel: Int = 1,
    val allyLevel: Int = 1
)
