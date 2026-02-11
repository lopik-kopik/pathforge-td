#include "Game.h"
#include "AndroidOut.h"
#include "GameStorage.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

Game::Game()
    : gold_(50), baseHP_(20), state_(GameState::LOGIN),
      waveTimer_(3.0f), spawnTimer_(0), enemiesSpawned_(0),
      totalEnemiesInWave_(0), waveActive_(false) {
    // Start with a short delay before first wave
    srand(static_cast<unsigned>(time(nullptr)));
    
    // Load saved menu coins
    menuCoins_ = GameStorage::loadMenuCoins();
}

void Game::update(float dt) {
    if (state_ == GameState::LOGIN || state_ == GameState::MENU || state_ == GameState::PAUSED || 
        state_ == GameState::GAME_OVER || state_ == GameState::VICTORY) {
        // Still update effects even when paused/game over
        damageNumbers_.update(dt);
        particles_.update(dt);
        return;
    }

    updateWaveSpawning(dt);
    updateEnemies(dt);
    updateTowers(dt);
    updateSheriffTowers(dt);
    updateMageTowers(dt);
    updateAllies(dt);
    updateProjectiles(dt);
    cleanupDead();
    damageNumbers_.update(dt);
    particles_.update(dt);

    // Check game over
    if (baseHP_ <= 0) {
        baseHP_ = 0;
        state_ = GameState::GAME_OVER;
        aout << "GAME OVER!" << std::endl;
        return;
    }

    // Check if wave is complete (all enemies spawned and dead/gone)
    if (waveActive_ && enemiesSpawned_ >= totalEnemiesInWave_ && enemies_.empty()) {
        waveActive_ = false;
        waveManager_.advanceWave();

        // In endless mode, never end - just keep generating waves
        if (difficulty_ != Difficulty::ENDLESS && waveManager_.allWavesDone()) {
            state_ = GameState::VICTORY;
            claimVictoryReward(); // Give menu coins for victory
            aout << "VICTORY!" << std::endl;
            return;
        }

        // Check for endless mode wave reward (every 10 waves)
        if (difficulty_ == Difficulty::ENDLESS) {
            checkEndlessWaveReward();
        }
        
        state_ = GameState::WAVE_COMPLETE;
        waveTimer_ = WAVE_DELAY;
        aout << "Wave complete! Next wave in " << WAVE_DELAY << " seconds." << std::endl;
    }
}

void Game::updateWaveSpawning(float dt) {
    if (state_ == GameState::WAVE_COMPLETE) {
        waveTimer_ -= dt;
        if (waveTimer_ <= 0) {
            startNextWave();
        }
        return;
    }

    if (!waveActive_) return;

    if (enemiesSpawned_ < totalEnemiesInWave_) {
        spawnTimer_ -= dt;
        if (spawnTimer_ <= 0) {
            spawnEnemy(spawnQueue_[enemiesSpawned_]);
            enemiesSpawned_++;
            spawnTimer_ = waveManager_.getCurrentWave().spawnInterval;
        }
    }
}

void Game::startNextWave() {
    if (difficulty_ != Difficulty::ENDLESS && waveManager_.allWavesDone()) return;

    state_ = GameState::PLAYING;
    waveActive_ = true;
    enemiesSpawned_ = 0;
    spawnTimer_ = 0; // spawn first enemy immediately

    buildSpawnQueue();
    totalEnemiesInWave_ = (int)spawnQueue_.size();

    if (difficulty_ == Difficulty::ENDLESS) {
        aout << "Starting endless wave " << endlessWaveNumber_ << std::endl;
    } else {
        aout << "Starting wave " << (waveManager_.getCurrentWaveIndex() + 1) << std::endl;
    }
}

void Game::buildSpawnQueue() {
    spawnQueue_.clear();
    
    if (difficulty_ == Difficulty::ENDLESS) {
        // Generate endless wave
        Wave wave = waveManager_.generateEndlessWave(endlessWaveNumber_);
        endlessWaveMultiplier_ = wave.difficultyMultiplier; // Save multiplier for this wave
        for (const auto& entry : wave.entries) {
            for (int i = 0; i < entry.count; i++) {
                spawnQueue_.push_back(entry.enemyType);
            }
        }
        endlessWaveNumber_++;
    } else {
        const Wave& wave = waveManager_.getCurrentWave();
        for (const auto& entry : wave.entries) {
            for (int i = 0; i < entry.count; i++) {
                spawnQueue_.push_back(entry.enemyType);
            }
        }
    }
}

void Game::spawnEnemy(EnemyType type) {
    enemies_.emplace_back(type, map_.getWaypoints());
    
    // Apply difficulty multiplier
    int multiplier;
    if (difficulty_ == Difficulty::ENDLESS) {
        multiplier = endlessWaveMultiplier_; // Use wave-specific multiplier
    } else {
        multiplier = getDifficultyMultiplier(); // Use global difficulty
    }
    
    if (multiplier != 100) {
        enemies_.back().applyDifficulty(multiplier);
    }
    
    if (type == EnemyType::BOSS) {
        int bossHP = 1500 * multiplier / 100;
        aout << "BOSS SPAWNED! HP: " << bossHP << std::endl;
    }
}

void Game::updateEnemies(float dt) {
    for (auto& enemy : enemies_) {
        enemy.update(dt);

        if (enemy.reachedEnd() && enemy.isAlive()) {
            baseHP_--;
            // Mark as dead and start death animation so it gets cleaned up
            enemy.takeDamage(9999);
        }

        // Grant kill reward exactly once for any death source (mages, projectiles, burn, etc.).
        // Escaped enemies (reachedEnd) should not award gold.
        if (enemy.isDying() && !enemy.reachedEnd() && !enemy.isRewardGranted()) {
            gold_ += enemy.getReward();
            enemy.markRewardGranted();
            spawnBlood(enemy.getX(), enemy.getY());
        }
    }
}

void Game::updateTowers(float dt) {
    for (auto& tower : towers_) {
        tower.update(dt);
        tower.updateAnim(dt);

        if (tower.canFire()) {
            int targetIdx = tower.findTarget(enemies_);
            if (targetIdx >= 0) {
                projectiles_.emplace_back(
                    tower.getX(), tower.getY(),
                    targetIdx, tower.getDamage());
                tower.resetCooldown();
                tower.triggerAttackAnim();
            }
        }
    }
}

void Game::updateSheriffTowers(float dt) {
    for (auto& sheriff : sheriffTowers_) {
        sheriff.update(dt, enemies_, projectiles_, map_.getWaypoints(), allies_);
    }
}

void Game::updateMageTowers(float dt) {
    for (auto& mage : mageTowers_) {
        mage.update(dt, enemies_, projectiles_);
    }
}

void Game::updateAllies(float dt) {
    for (auto& ally : allies_) {
        if (ally.isAlive()) {
            ally.update(dt, enemies_, this);
        }
    }

    // Check for enemies attacking allies
    for (auto& enemy : enemies_) {
        if (!enemy.isAlive()) continue;
        for (auto& ally : allies_) {
            if (!ally.isAlive()) continue;
            float dx = enemy.getX() - ally.getX();
            float dy = enemy.getY() - ally.getY();
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 0.5f) {
                // Enemy attacks ally
                int damage = (enemy.getType() == EnemyType::GOBLIN) ? 35 : 
                             (enemy.getType() == EnemyType::SLIME) ? 50 : 40;
                ally.takeDamage(damage);
            }
        }
    }
}

void Game::updateProjectiles(float dt) {
    for (auto& proj : projectiles_) {
        if (!proj.isActive()) continue;

        int targetIdx = proj.getTargetIndex();

        // Check if target is still valid
        if (targetIdx < 0 || targetIdx >= (int)enemies_.size() ||
            !enemies_[targetIdx].isAlive()) {
            proj.deactivate();
            continue;
        }

        Enemy& target = enemies_[targetIdx];
        proj.update(dt, target.getX(), target.getY());

        if (proj.hasReachedTarget(target.getX(), target.getY())) {
            bool wasAlive = target.isAlive();
            int damage = proj.getDamage();
            target.takeDamage(damage);
            
            // Spawn damage number
            if (damage > 0) {
                spawnDamageNumber(target.getX(), target.getY() + 0.3f, damage);
            }

            // Death and gold reward now handled in updateEnemies()
            if (wasAlive && !target.isAlive()) {
                spawnExplosion(target.getX(), target.getY());
            }

            proj.deactivate();
        }
    }
}

void Game::cleanupDead() {
    // Remove dead or reached-spawn allies
    allies_.erase(
        std::remove_if(allies_.begin(), allies_.end(),
            [](const Ally& a) { return !a.isAlive() || a.reachedSpawn(); }),
        allies_.end());

    // Remove dead enemies (after death animation completes)
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [](const Enemy& e) { return e.isDead(); }),
        enemies_.end());

    // Remove inactive projectiles
    projectiles_.erase(
        std::remove_if(projectiles_.begin(), projectiles_.end(),
            [](const Projectile& p) { return !p.isActive(); }),
        projectiles_.end());

    // After removing enemies, projectile target indices are invalid!
    // We need to deactivate projectiles whose targets are gone
    // Actually, let's fix this: use a simpler approach where projectiles
    // track position, not index, after target removal
    // For now, deactivate all projectiles targeting removed enemies
    for (auto& proj : projectiles_) {
        int idx = proj.getTargetIndex();
        if (idx >= (int)enemies_.size()) {
            proj.deactivate();
        }
    }

    // Clean up again after deactivating stale projectiles
    projectiles_.erase(
        std::remove_if(projectiles_.begin(), projectiles_.end(),
            [](const Projectile& p) { return !p.isActive(); }),
        projectiles_.end());
}

void Game::onTap(float worldX, float worldY) {
    if (state_ == GameState::GAME_OVER || state_ == GameState::MENU)
        return;
    
    // Victory screen - check for menu button tap
    if (state_ == GameState::VICTORY) {
        float menuBtnWidth = 3.0f;
        float menuBtnHeight = 1.0f;
        float menuBtnX = Map::COLS * 0.5f - menuBtnWidth * 0.5f;
        float menuBtnY = Map::ROWS * 0.45f;
        if (worldX >= menuBtnX && worldX <= menuBtnX + menuBtnWidth &&
            worldY >= menuBtnY && worldY <= menuBtnY + menuBtnHeight) {
            returnToMenu();
        }
        return;
    }

    // If paused, resume on tap
    if (state_ == GameState::PAUSED) {
        resumeGame();
        return;
    }

    // If upgrade menu is open, check if tapped on upgrade button or close it
    if (isUpgradeMenuOpen()) {
        // Simple check: tap anywhere closes menu (upgrade button handled separately)
        // Actually, let HUD handle this - just pass the tap through
        closeUpgradeMenu();
        return;
    }

    int col = (int)worldX;
    int row = (int)worldY;

    // Bounds check
    if (col < 0 || col >= Map::COLS || row < 0 || row >= Map::ROWS)
        return;

    // Check if tapped on existing tower
    for (int i = 0; i < (int)towers_.size(); i++) {
        if (towers_[i].getGridCol() == col && towers_[i].getGridRow() == row) {
            // Tapped on existing tower - open upgrade menu
            openUpgradeMenu(i);
            aout << "Tower selected at (" << col << ", " << row << ") Level: " << towers_[i].getLevel() << std::endl;
            return;
        }
    }

    // Try to place new tower based on selected type
    if (map_.canPlaceTower(col, row)) {
        if (selectedTowerType_ == TowerType::SHERIFF && (gold_ >= SheriffTower::getCost(difficulty_) || sandboxMode_)) {
            // Place Sheriff tower
            sheriffTowers_.emplace_back(col, row);
            map_.placeTower(col, row);
            if (!sandboxMode_) gold_ -= SheriffTower::getCost(difficulty_);
            aout << "Sheriff Tower placed at (" << col << ", " << row << "). Gold: " << gold_ << std::endl;
        } else if (selectedTowerType_ == TowerType::MAGE && (gold_ >= MageTower::getCost(difficulty_) || sandboxMode_)) {
            // Place Mage tower
            mageTowers_.emplace_back(col, row, selectedMageElement_);
            map_.placeTower(col, row);
            if (!sandboxMode_) gold_ -= MageTower::getCost(difficulty_);
            aout << "Mage Tower (" << (int)selectedMageElement_ << ") placed at (" << col << ", " << row << "). Gold: " << gold_ << std::endl;
        } else if (selectedTowerType_ == TowerType::ARCHER && (gold_ >= Tower::getCost(difficulty_) || sandboxMode_)) {
            // Place regular tower
            towers_.emplace_back(col, row);
            map_.placeTower(col, row);
            if (!sandboxMode_) gold_ -= Tower::getCost(difficulty_);
            aout << "Tower placed at (" << col << ", " << row << "). Gold: " << gold_ << std::endl;
        }
    }
}

void Game::openUpgradeMenu(int towerIndex) {
    selectedTowerIndex_ = towerIndex;
}

void Game::startGame() {
    if (state_ == GameState::MENU) {
        state_ = GameState::DIFFICULTY_SELECT;
        aout << "Select difficulty" << std::endl;
    }
}

void Game::startGameWithDifficulty(Difficulty diff) {
    difficulty_ = diff;
    state_ = GameState::WAVE_COMPLETE;
    waveTimer_ = 3.0f;
    
    // Adjust starting gold based on difficulty
    if (diff == Difficulty::EASY) {
        gold_ = 75;
        baseHP_ = 25;
    } else if (diff == Difficulty::HARD) {
        gold_ = 35;
        baseHP_ = 15;
    } else if (diff == Difficulty::ENDLESS) {
        gold_ = 50;
        baseHP_ = 15;
    } else {
        gold_ = 50;
        baseHP_ = 20;
    }
    
    aout << "Game started! Difficulty: " << (int)diff << std::endl;
}

int Game::getDifficultyMultiplier() const {
    switch (difficulty_) {
        case Difficulty::EASY: return 75;   // 75% HP and speed
        case Difficulty::HARD: return 150;  // 150% HP and speed
        case Difficulty::ENDLESS: return 200; // 200% HP for endless
        default: return 100;                // 100% (normal)
    }
}

void Game::togglePause() {
    if (state_ == GameState::PLAYING) {
        state_ = GameState::PAUSED;
        aout << "Game paused" << std::endl;
    } else if (state_ == GameState::PAUSED) {
        state_ = GameState::PLAYING;
        aout << "Game resumed" << std::endl;
    }
}

void Game::restartGame() {
    // Reset everything
    gold_ = 50;
    baseHP_ = 20;
    state_ = GameState::WAVE_COMPLETE;
    waveTimer_ = 3.0f;
    spawnTimer_ = 0;
    enemiesSpawned_ = 0;
    totalEnemiesInWave_ = 0;
    waveActive_ = false;
    spawnQueue_.clear();
    enemies_.clear();
    towers_.clear();
    sheriffTowers_.clear();
    mageTowers_.clear();
    allies_.clear();
    projectiles_.clear();
    map_ = Map(); // Reset map
    waveManager_ = WaveManager(); // Reset waves
    selectedTowerIndex_ = -1;
    endlessWaveNumber_ = 1; // Reset endless wave counter
    endlessWaveMultiplier_ = 100; // Reset multiplier
    endlessRewardCounter_ = 0; // Reset reward counter
    aout << "Game restarted!" << std::endl;
}

void Game::returnToMenu() {
    restartGame();
    state_ = GameState::MENU;
    aout << "Returned to menu" << std::endl;
}

void Game::claimVictoryReward() {
    int reward = 0;
    switch (difficulty_) {
        case Difficulty::EASY:
            reward = 50;
            break;
        case Difficulty::MEDIUM:
            reward = 100;
            break;
        case Difficulty::HARD:
            reward = 250;
            break;
        default:
            reward = 0;
    }
    if (reward > 0) {
        menuCoins_ += reward;
        GameStorage::saveMenuCoins(menuCoins_);
        aout << "Victory! Earned " << reward << " menu coins. Total: " << menuCoins_ << std::endl;
    }
}

void Game::checkEndlessWaveReward() {
    if (difficulty_ != Difficulty::ENDLESS) return;
    
    endlessRewardCounter_++;
    if (endlessRewardCounter_ >= 10) {
        menuCoins_ += 100;
        GameStorage::saveMenuCoins(menuCoins_);
        aout << "Endless wave reward! Earned 100 menu coins. Total: " << menuCoins_ << std::endl;
        endlessRewardCounter_ = 0; // Reset counter
    }
}

void Game::addMenuCoins(int amount) {
    menuCoins_ += amount;
    GameStorage::saveMenuCoins(menuCoins_);
}

void Game::spendMenuCoins(int amount) {
    menuCoins_ -= amount;
    GameStorage::saveMenuCoins(menuCoins_);
}

const Tower* Game::getSelectedTower() const {
    if (selectedTowerIndex_ >= 0 && selectedTowerIndex_ < (int)towers_.size()) {
        return &towers_[selectedTowerIndex_];
    }
    return nullptr;
}

bool Game::upgradeSelectedTower() {
    if (selectedTowerIndex_ < 0 || selectedTowerIndex_ >= (int)towers_.size()) {
        return false;
    }
    
    Tower& tower = towers_[selectedTowerIndex_];
    if (!tower.canUpgrade(gold_)) {
        return false;
    }
    
    int cost = tower.getUpgradeCost();
    gold_ -= cost;
    tower.upgrade();
    
    aout << "Tower upgraded to level " << tower.getLevel() << "! Damage: " << tower.getDamage() 
         << ", Range: " << tower.getRange() << ", FireRate: " << tower.getFireRate() << std::endl;
    
    // Close menu after upgrade
    closeUpgradeMenu();
    return true;
}

int Game::getUpgradeCost(int currentLevel) const {
    // Cost progression: 10, 30, 50, 70, 90, 110...
    // Formula: 10 + (level - 1) * 20
    return 10 + (currentLevel - 1) * 20;
}

bool Game::upgradeArcher() {
    int cost = getUpgradeCost(archerLevel_);
    if (cards_ < cost) return false;
    
    cards_ -= cost;
    archerLevel_++;
    aout << "Archer upgraded to level " << archerLevel_ << "! Cost: " << cost << " cards" << std::endl;
    return true;
}

bool Game::upgradeSheriff() {
    int cost = getUpgradeCost(sheriffLevel_);
    if (cards_ < cost) return false;
    
    cards_ -= cost;
    sheriffLevel_++;
    aout << "Sheriff upgraded to level " << sheriffLevel_ << "! Cost: " << cost << " cards" << std::endl;
    return true;
}

bool Game::upgradeAlly() {
    int cost = getUpgradeCost(allyLevel_);
    if (cards_ < cost) return false;
    
    cards_ -= cost;
    allyLevel_++;
    aout << "Ally upgraded to level " << allyLevel_ << "! Cost: " << cost << " cards" << std::endl;
    return true;
}

bool Game::buyChest() {
    const int CHEST_COST = 200;
    if (menuCoins_ < CHEST_COST) return false;
    
    menuCoins_ -= CHEST_COST;
    GameStorage::saveMenuCoins(menuCoins_);
    aout << "Chest purchased! Remaining coins: " << menuCoins_ << std::endl;
    return true;
}

void Game::openChest() {
    // Random cards: 50-150 cards per chest
    int cards = 50 + (rand() % 101); // 50 to 150
    cards_ += cards;
    aout << "Chest opened! Got " << cards << " cards. Total: " << cards_ << std::endl;
}


// ============================================
// SANDBOX MODE
// ============================================

void Game::enterSandbox() {
    state_ = GameState::SANDBOX;
    sandboxMode_ = true;
    sandboxInfiniteMoney_ = true;
    gold_ = 9999;
    baseHP_ = 999;
    sandboxWave_ = 1;
    
    // Clear any existing game state
    enemies_.clear();
    projectiles_.clear();
    waveActive_ = false;
    
    aout << "=== SANDBOX MODE ===" << std::endl;
    aout << "Infinite gold! Test your builds!" << std::endl;
    aout << "Controls: BUILD towers, NEXT WAVE to test, CLEAR to reset" << std::endl;
}

void Game::exitSandbox() {
    returnToMenu();
    sandboxMode_ = false;
    sandboxInfiniteMoney_ = false;
}

void Game::sandboxClearTowers() {
    towers_.clear();
    sheriffTowers_.clear();
    mageTowers_.clear();
    allies_.clear();
    enemies_.clear();
    projectiles_.clear();
    
    // Reset map
    map_ = Map();
    
    aout << "Sandbox: All towers cleared!" << std::endl;
}

void Game::sandboxSpawnWave(int waveNumber) {
    // Generate a test wave
    Wave wave = waveManager_.generateEndlessWave(waveNumber);
    
    spawnQueue_.clear();
    for (const auto& entry : wave.entries) {
        for (int i = 0; i < entry.count; i++) {
            spawnQueue_.push_back(entry.enemyType);
        }
    }
    
    totalEnemiesInWave_ = (int)spawnQueue_.size();
    enemiesSpawned_ = 0;
    waveActive_ = true;
    spawnTimer_ = 0;
    
    aout << "Sandbox: Spawning wave " << waveNumber << " with " << totalEnemiesInWave_ << " enemies" << std::endl;
}

void Game::sandboxSpawnEnemy(EnemyType type) {
    // Spawn single enemy immediately
    spawnEnemy(type);
    aout << "Sandbox: Spawned " << (int)type << std::endl;
}

void Game::sandboxSetGold(int amount) {
    gold_ = amount;
    aout << "Sandbox: Gold set to " << amount << std::endl;
}

void Game::sandboxNextWave() {
    sandboxSpawnWave(sandboxWave_);
    sandboxWave_++;
}

// Account system implementation
void Game::login(const std::string& email, const std::string& uid) {
    account_.email = email;
    account_.uid = uid;
    account_.isLoggedIn = true;
    
    // Load cloud data (this would sync from server in real implementation)
    // For now, merge local and account data
    account_.menuCoins = menuCoins_;
    account_.cards = cards_;
    account_.archerLevel = archerLevel_;
    account_.sheriffLevel = sheriffLevel_;
    account_.allyLevel = allyLevel_;
    
    aout << "Logged in as: " << email << std::endl;
    state_ = GameState::MENU;
}

void Game::logout() {
    // Save current progress before logout
    syncAccountData();
    
    account_.clear();
    aout << "Logged out" << std::endl;
    state_ = GameState::LOGIN;
}

void Game::syncAccountData() {
    if (!account_.isLoggedIn) return;
    
    // Update account data from local
    account_.menuCoins = menuCoins_;
    account_.cards = cards_;
    account_.archerLevel = archerLevel_;
    account_.sheriffLevel = sheriffLevel_;
    account_.allyLevel = allyLevel_;
    
    // In real implementation, this would upload to cloud server
    // For now, we save locally
    GameStorage::saveMenuCoins(menuCoins_);
    
    aout << "Account data synced" << std::endl;
}

void Game::loadAccountData(const Account& data) {
    account_ = data;
    
    // Update local game data from account
    menuCoins_ = data.menuCoins;
    cards_ = data.cards;
    archerLevel_ = data.archerLevel;
    sheriffLevel_ = data.sheriffLevel;
    allyLevel_ = data.allyLevel;
    
    aout << "Account data loaded for: " << data.email << std::endl;
}

void Game::spawnDamageNumber(float x, float y, int damage, bool critical, bool burn) {
    damageNumbers_.spawn(x, y, damage, critical, burn);
}
