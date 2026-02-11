package com.pathforge.core.domain

enum class CellType {
    GRASS,
    PATH,
    BLOCKED,
    TOWER
}

data class Waypoint(val x: Float, val y: Float)

class GameMap {
    companion object {
        const val COLS = 10
        const val ROWS = 16
    }

    private val grid = Array(COLS) { Array(ROWS) { CellType.GRASS } }

    val waypoints: List<Waypoint> = listOf(
        Waypoint(5.5f, 15.5f),
        Waypoint(5.5f, 11.5f),
        Waypoint(8.5f, 11.5f),
        Waypoint(8.5f, 8.5f),
        Waypoint(1.5f, 8.5f),
        Waypoint(1.5f, 5.5f),
        Waypoint(7.5f, 5.5f),
        Waypoint(7.5f, 2.5f),
        Waypoint(4.5f, 2.5f),
        Waypoint(4.5f, 0.5f)
    )

    private val pathCells = listOf(
        5 to 15, 5 to 14, 5 to 13, 5 to 12, 5 to 11,
        6 to 11, 7 to 11, 8 to 11,
        8 to 10, 8 to 9, 8 to 8,
        7 to 8, 6 to 8, 5 to 8, 4 to 8, 3 to 8, 2 to 8, 1 to 8,
        1 to 7, 1 to 6, 1 to 5,
        2 to 5, 3 to 5, 4 to 5, 5 to 5, 6 to 5, 7 to 5,
        7 to 4, 7 to 3, 7 to 2,
        6 to 2, 5 to 2, 4 to 2,
        4 to 1, 4 to 0
    )

    private val blockedCells = listOf(
        0 to 15, 1 to 15, 9 to 15,
        0 to 12, 3 to 13, 9 to 13,
        0 to 9, 3 to 10, 6 to 13,
        9 to 6, 9 to 3, 0 to 3,
        0 to 0, 1 to 0, 9 to 0
    )

    init {
        reset()
    }

    fun getCell(col: Int, row: Int): CellType {
        if (col !in 0 until COLS || row !in 0 until ROWS) return CellType.BLOCKED
        return grid[col][row]
    }

    fun canPlaceTower(col: Int, row: Int): Boolean = getCell(col, row) == CellType.GRASS

    fun placeTower(col: Int, row: Int) {
        if (col in 0 until COLS && row in 0 until ROWS && grid[col][row] == CellType.GRASS) {
            grid[col][row] = CellType.TOWER
        }
    }

    fun reset() {
        fill(CellType.GRASS)
        pathCells.forEach { (x, y) -> grid[x][y] = CellType.PATH }
        blockedCells.forEach { (x, y) -> grid[x][y] = CellType.BLOCKED }
    }

    private fun fill(cellType: CellType) {
        for (x in 0 until COLS) {
            for (y in 0 until ROWS) {
                grid[x][y] = cellType
            }
        }
    }
}
