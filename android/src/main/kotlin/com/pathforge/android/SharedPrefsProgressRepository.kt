package com.pathforge.android

import android.content.Context
import com.pathforge.core.domain.ProgressData
import com.pathforge.core.persistence.ProgressRepository

class SharedPrefsProgressRepository(context: Context) : ProgressRepository {
    private val prefs = context.getSharedPreferences("pathforge_progress", Context.MODE_PRIVATE)

    override fun load(): ProgressData {
        val raw = prefs.getString("progress_v2", null) ?: return ProgressData()
        val parts = raw.split("|")
        if (parts.size != 6) return ProgressData()
        return ProgressData(
            schemaVersion = parts[0].toIntOrNull() ?: 2,
            menuCoins = parts[1].toIntOrNull() ?: 0,
            cards = parts[2].toIntOrNull() ?: 0,
            archerLevel = parts[3].toIntOrNull() ?: 1,
            sheriffLevel = parts[4].toIntOrNull() ?: 1,
            allyLevel = parts[5].toIntOrNull() ?: 1
        )
    }

    override fun save(data: ProgressData) {
        val encoded = listOf(
            data.schemaVersion,
            data.menuCoins,
            data.cards,
            data.archerLevel,
            data.sheriffLevel,
            data.allyLevel
        ).joinToString("|")

        prefs.edit().putString("progress_v2", encoded).apply()
    }
}
