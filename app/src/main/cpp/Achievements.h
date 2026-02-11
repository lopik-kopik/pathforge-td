#ifndef TOWERDEFENSE_ACHIEVEMENTS_H
#define TOWERDEFENSE_ACHIEVEMENTS_H

#include <vector>
#include <string>

enum class AchievementType {
    KILL_SLIMES,        // Убить 1000 слаймов
    KILL_GOBLINS,       // Убить 500 гоблинов
    KILL_BOSSES,        // Убить 50 боссов
    NO_SHERIFF_WIN,     // Пройти без Sheriff башен
    ARCHER_ONLY_WIN,    // Пройти только Archer
    ENDLESS_50,         // Выжить 50 волн в Endless
    MAX_TOWER_LEVEL,    // Вкачать башню до макс уровня
    RICH_PLAYER,        // Накопить 1000 золота
    CHEST_COLLECTOR,    // Открыть 10 сундуков
    UPGRADE_MASTER      // Вкачать всех персонажей до 5 уровня
};

struct Achievement {
    AchievementType type;
    const char* name;
    const char* description;
    int target;
    int current;
    bool unlocked;
    int reward; // Menu coins reward
};

class AchievementManager {
public:
    AchievementManager();
    
    void init();
    void load(); // Load from storage
    void save(); // Save to storage
    
    void progress(AchievementType type, int amount = 1);
    void unlock(AchievementType type);
    bool isUnlocked(AchievementType type) const;
    
    const std::vector<Achievement>& getAll() const { return achievements_; }
    int getUnlockedCount() const;
    int getTotalReward() const;
    
    // Stat tracking
    int totalSlimesKilled = 0;
    int totalGoblinsKilled = 0;
    int totalBossesKilled = 0;
    int chestsOpened = 0;
    int maxGold = 0;
    int endlessWaves = 0;
    
private:
    std::vector<Achievement> achievements_;
    
    void addAchievement(AchievementType type, const char* name, const char* desc, int target, int reward);
};

#endif //TOWERDEFENSE_ACHIEVEMENTS_H
