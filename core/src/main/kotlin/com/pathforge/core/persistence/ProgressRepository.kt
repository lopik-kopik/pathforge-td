package com.pathforge.core.persistence

import com.pathforge.core.domain.ProgressData

interface ProgressRepository {
    fun load(): ProgressData
    fun save(data: ProgressData)
}
