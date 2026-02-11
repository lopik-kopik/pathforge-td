package com.pathforge.core.domain

enum class TowerType {
    ARCHER,
    SHERIFF,
    MAGE
}

enum class EnemyType {
    SLIME,
    GOBLIN,
    BAT,
    BOSS
}

data class Tower(
    val id: Long,
    val gridX: Int,
    val gridY: Int,
    val type: TowerType,
    val level: Int = 1,
    var cooldown: Float = 0f,
    val fireRate: Float = 0.7f,
    val damage: Int = 12,
    val range: Float = 4.0f
)

fun towerUpgradeCost(level: Int): Int {
    return when (level) {
        1 -> 30
        2 -> 50
        3 -> 100
        4 -> 200
        else -> 0
    }
}

fun towerMaxLevel(): Int = 5

data class Enemy(
    val id: Long,
    val type: EnemyType,
    var x: Float,
    var y: Float,
    var hp: Int,
    val speed: Float,
    val reward: Int,
    var waypointIndex: Int = 1,
    var reachedEnd: Boolean = false
)

data class Projectile(
    val id: Long,
    var x: Float,
    var y: Float,
    val targetId: Long,
    val damage: Int,
    val speed: Float = 10f
)
