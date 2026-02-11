#include "Achievements.h"
#include <algorithm>

AchievementManager::AchievementManager() {
    init();
}

void AchievementManager::init() {
    achievements_.clear();
    
    addAchievement(AchievementType::KILL_SLIMES, 
        "Slime Hunter", "Kill 1000 slimes", 1000, 100);
    addAchievement(AchievementType::KILL_GOBLINS,
        "Goblin Slayer", "Kill 500 goblins", 500, 150);
    addAchievement(AchievementType::KILL_BOSSES,
        "Boss Crusher", "Kill 50 bosses", 50, 500);
    addAchievement(AchievementType::NO_SHERIFF_WIN,
        "No Sheriff Needed", "Win without Sheriff towers", 1, 200);
    addAchievement(AchievementType::ARCHER_ONLY_WIN,
        "Archer Master", "Win using only Archer towers", 1, 300);
    addAchievement(AchievementType::ENDLESS_50,
        "Endless Survivor", "Survive 50 waves in Endless", 50, 250);
    addAchievement(AchievementType::MAX_TOWER_LEVEL,
        "Max Power", "Upgrade a tower to max level", 1, 100);
    addAchievement(AchievementType::RICH_PLAYER,
        "Millionaire", "Have 1000 gold at once", 1000, 50);
    addAchievement(AchievementType::CHEST_COLLECTOR,
        "Treasure Hunter", "Open 10 chests", 10, 100);
    addAchievement(AchievementType::UPGRADE_MASTER,
        "Upgrade Master", "Get all characters to level 5", 3, 500);
}

void AchievementManager::addAchievement(AchievementType type, const char* name, 
                                       const char* desc, int target, int reward) {
    achievements_.push_back({type, name, desc, target, 0, false, reward});
}

void AchievementManager::progress(AchievementType type, int amount) {
    auto it = std::find_if(achievements_.begin(), achievements_.end(),
        [type](const Achievement& a) { return a.type == type; });
    
    if (it != achievements_.end() && !it->unlocked) {
        it->current += amount;
        if (it->current >= it->target) {
            unlock(type);
        }
    }
}

void AchievementManager::unlock(AchievementType type) {
    auto it = std::find_if(achievements_.begin(), achievements_.end(),
        [type](const Achievement& a) { return a.type == type; });
    
    if (it != achievements_.end() && !it->unlocked) {
        it->unlocked = true;
        it->current = it->target;
        // TODO: Show notification
        // TODO: Add reward to menu coins
    }
}

bool AchievementManager::isUnlocked(AchievementType type) const {
    auto it = std::find_if(achievements_.begin(), achievements_.end(),
        [type](const Achievement& a) { return a.type == type; });
    return it != achievements_.end() && it->unlocked;
}

int AchievementManager::getUnlockedCount() const {
    return std::count_if(achievements_.begin(), achievements_.end(),
        [](const Achievement& a) { return a.unlocked; });
}

int AchievementManager::getTotalReward() const {
    int total = 0;
    for (const auto& a : achievements_) {
        if (a.unlocked) total += a.reward;
    }
    return total;
}

void AchievementManager::load() {
    // TODO: Load from SharedPreferences or file
}

void AchievementManager::save() {
    // TODO: Save to SharedPreferences or file
}
