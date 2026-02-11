package com.pathforge.game.persistence

import com.pathforge.core.domain.ProgressData
import com.pathforge.core.persistence.ProgressRepository

class InMemoryProgressRepository : ProgressRepository {
    private var data = ProgressData()

    override fun load(): ProgressData = data

    override fun save(data: ProgressData) {
        this.data = data
    }
}
