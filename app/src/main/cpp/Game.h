#ifndef TOWERDEFENSE_GAME_H
#define TOWERDEFENSE_GAME_H

#include <vector>
#include "Map.h"
#include "Tower.h"
#include "SheriffTower.h"
#include "MageTower.h"
#include "Ally.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Wave.h"
#include "DamageNumber.h"
#include "ParticleSystem.h"
#include "Achievements.h"

enum class GameState {
    LOGIN,          // Login/Register screen
    MENU,           // Main menu
    DIFFICULTY_SELECT, // Difficulty selection
    CHARACTER_MENU, // Character upgrade menu
    SANDBOX,        // Sandbox mode (infinite money, testing)
    PLAYING,
    PAUSED,         // Game paused
    WAVE_COMPLETE,
    GAME_OVER,
    VICTORY
};

enum class Difficulty {
    EASY = 0,
    MEDIUM = 1,
    HARD = 2,
    ENDLESS = 3
};

// Account data structure
struct Account {
    std::string email;
    std::string uid;
    bool isLoggedIn = false;
    
    // Cloud-synced data
    int menuCoins = 0;
    int cards = 0;
    int archerLevel = 1;
    int sheriffLevel = 1;
    int allyLevel = 1;
    
    void clear() {
        email.clear();
        uid.clear();
        isLoggedIn = false;
        menuCoins = 0;
        cards = 0;
        archerLevel = 1;
        sheriffLevel = 1;
        allyLevel = 1;
    }
};

class Game {
public:
    Game();

    void update(float dt);
    void onTap(float worldX, float worldY);

    // Accessors for rendering
    const Map& getMap() const { return map_; }
    const std::vector<Tower>& getTowers() const { return towers_; }
    const std::vector<SheriffTower>& getSheriffTowers() const { return sheriffTowers_; }
    const std::vector<MageTower>& getMageTowers() const { return mageTowers_; }
    const std::vector<Ally>& getAllies() const { return allies_; }
    const std::vector<Enemy>& getEnemies() const { return enemies_; }
    const std::vector<Projectile>& getProjectiles() const { return projectiles_; }
    DamageNumberManager& getDamageNumbers() { return damageNumbers_; }

    int getGold() const { return gold_; }
    void addGold(int amount) { gold_ += amount; }
    int getBaseHP() const { return baseHP_; }
    int getCurrentWave() const { return waveManager_.getCurrentWaveIndex(); }
    int getTotalWaves() const { return waveManager_.getTotalWaves(); }
    int getDisplayedWave() const { return difficulty_ == Difficulty::ENDLESS ? endlessWaveNumber_ - 1 : waveManager_.getCurrentWaveIndex() + 1; }
    GameState getState() const { return state_; }
    
    // Menu coins (persistent between games)
    int getMenuCoins() const { return menuCoins_; }
    void addMenuCoins(int amount);
    void spendMenuCoins(int amount);
    
    // Claim reward after victory/endless waves
    void claimVictoryReward();
    void checkEndlessWaveReward(); // Call when wave completes
    
    // Character upgrade system (cards)
    int getCards() const { return cards_; }
    void addCards(int amount) { cards_ += amount; }
    
    // Character levels
    int getArcherLevel() const { return archerLevel_; }
    int getSheriffLevel() const { return sheriffLevel_; }
    int getAllyLevel() const { return allyLevel_; }
    
    // Account system
    const Account& getAccount() const { return account_; }
    bool isLoggedIn() const { return account_.isLoggedIn; }
    void login(const std::string& email, const std::string& uid);
    void logout();
    void syncAccountData(); // Sync local data with cloud
    void loadAccountData(const Account& data);
    
    // Login UI helpers
    void openLoginScreen() { state_ = GameState::LOGIN; }
    void skipLogin() { state_ = GameState::MENU; } // Guest mode
    
    // Upgrade costs (10, 30, 50, 70, 90...)
    int getUpgradeCost(int currentLevel) const;
    
    // Upgrade characters
    bool upgradeArcher();
    bool upgradeSheriff();
    bool upgradeAlly();
    
    // Chest system
    bool buyChest(); // Returns true if purchase successful
    void openChest(); // Adds random cards
    
    // Navigation
    void openCharacterMenu() { state_ = GameState::CHARACTER_MENU; }
    void closeCharacterMenu() { state_ = GameState::MENU; }
    bool isCharacterMenu() const { return state_ == GameState::CHARACTER_MENU; }
    
    // Map selection
    void setMapType(MapType type) { map_.setMapType(type); }
    MapType getMapType() const { return map_.getType(); }
    const char* getMapName() const { return map_.getMapName(); }
    
    // Sandbox mode
    void enterSandbox();
    void exitSandbox();
    bool isSandbox() const { return state_ == GameState::SANDBOX; }
    void sandboxClearTowers(); // Remove all towers
    void sandboxSpawnWave(int waveNumber); // Spawn specific wave
    void sandboxSpawnEnemy(EnemyType type); // Spawn single enemy
    void sandboxSetGold(int amount);
    int getSandboxWave() const { return sandboxWave_; }
    void sandboxNextWave();

    // Main menu
    void startGame();
    void startGameWithDifficulty(Difficulty diff);
    bool isInMenu() const { return state_ == GameState::MENU; }
    bool isDifficultySelect() const { return state_ == GameState::DIFFICULTY_SELECT; }
    
    // Difficulty
    void setDifficulty(Difficulty diff) { difficulty_ = diff; }
    Difficulty getDifficulty() const { return difficulty_; }
    int getDifficultyMultiplier() const; // Returns HP/speed multiplier percentage
    
    // Pause
    void togglePause();
    bool isPaused() const { return state_ == GameState::PAUSED; }
    void resumeGame() { if (state_ == GameState::PAUSED) state_ = GameState::PLAYING; }
    void restartGame();
    void returnToMenu();

    // Upgrade menu
    bool isUpgradeMenuOpen() const { return selectedTowerIndex_ >= 0; }
    int getSelectedTowerIndex() const { return selectedTowerIndex_; }
    const Tower* getSelectedTower() const;
    void openUpgradeMenu(int towerIndex);
    void closeUpgradeMenu() { selectedTowerIndex_ = -1; }
    bool upgradeSelectedTower();
    
    // Tower type selection
    int getSelectedTowerType() const { return static_cast<int>(selectedTowerType_); }
    void selectArcherTower() { selectedTowerType_ = TowerType::ARCHER; }
    void selectSheriffTower() { selectedTowerType_ = TowerType::SHERIFF; }
    void selectMageTower() { selectedTowerType_ = TowerType::MAGE; }
    
    // Mage element selection
    ElementType getSelectedMageElement() const { return selectedMageElement_; }
    void setMageElement(ElementType element) { selectedMageElement_ = element; }
    
    // Sandbox mode
    bool isSandboxMode() const { return sandboxMode_; }
    void toggleSandboxMode() { sandboxMode_ = !sandboxMode_; }

private:
    void updateTowers(float dt);
    void updateSheriffTowers(float dt);
    void updateMageTowers(float dt);
    void updateAllies(float dt);
    void updateEnemies(float dt);
    void updateProjectiles(float dt);
    void updateWaveSpawning(float dt);
    void cleanupDead();
    void spawnEnemy(EnemyType type);
    void startNextWave();

    // Flatten wave entries into a spawn queue
    void buildSpawnQueue();

    Map map_;
    std::vector<Tower> towers_;
    std::vector<SheriffTower> sheriffTowers_;
    std::vector<MageTower> mageTowers_;
    std::vector<Ally> allies_;
    std::vector<Enemy> enemies_;
    std::vector<Projectile> projectiles_;

    WaveManager waveManager_;

    int gold_;
    int baseHP_;
    GameState state_;

    // Wave spawning
    float waveTimer_;      // countdown between waves
    float spawnTimer_;     // countdown between enemy spawns
    int enemiesSpawned_;   // how many spawned in current wave
    int totalEnemiesInWave_;
    bool waveActive_;
    
    // Endless mode
    int endlessWaveNumber_ = 1;
    int endlessWaveMultiplier_ = 100; // Current wave HP multiplier
    int endlessRewardCounter_ = 0; // Track waves for reward (every 10)

    // Spawn queue for current wave (flattened from WaveEntries)
    std::vector<EnemyType> spawnQueue_;
    
    // Persistent menu coins
    int menuCoins_ = 0;
    
    // Account
    Account account_;
    
public:
    // Damage numbers (visual feedback)
    void spawnDamageNumber(float x, float y, int damage, bool critical = false, bool burn = false);
    
    // Particle effects
    void spawnExplosion(float x, float y) { particles_.spawnExplosion(x, y); }
    void spawnSpark(float x, float y) { particles_.spawnSpark(x, y); }
    void spawnBlood(float x, float y) { particles_.spawnBlood(x, y); }
    void spawnIce(float x, float y) { particles_.spawnIce(x, y); }
    void spawnFire(float x, float y) { particles_.spawnFire(x, y); }
    ParticleSystem& getParticles() { return particles_; }
    
    // Achievements
    AchievementManager& getAchievements() { return achievements_; }
    
private:
    // Sandbox mode (infinite money for testing)
    bool sandboxMode_ = false;
    int sandboxWave_ = 1; // Current sandbox wave for display
    bool sandboxInfiniteMoney_ = false;
    
    DamageNumberManager damageNumbers_;
    ParticleSystem particles_;
    AchievementManager achievements_;
    
    // Cards for character upgrades
    int cards_ = 0;
    
    // Character upgrade levels (affect starting stats in new games)
    int archerLevel_ = 1;
    int sheriffLevel_ = 1;
    int allyLevel_ = 1;

    static constexpr float WAVE_DELAY = 5.0f;

    // Upgrade menu
    int selectedTowerIndex_ = -1; // -1 = no tower selected
    
    // Difficulty
    Difficulty difficulty_ = Difficulty::MEDIUM;
    
    // Tower placement mode
    enum class TowerType { ARCHER, SHERIFF, MAGE };
    TowerType selectedTowerType_ = TowerType::ARCHER;
    
    // Mage element selection
    ElementType selectedMageElement_ = ElementType::FIRE;
};

#endif //TOWERDEFENSE_GAME_H
