#ifndef TOWERDEFENSE_GAMESTORAGE_H
#define TOWERDEFENSE_GAMESTORAGE_H

#include <string>

// Save/load game data using Android SharedPreferences and Firebase
class GameStorage {
public:
    // Local storage for coins
    static void saveMenuCoins(int coins);
    static int loadMenuCoins();
    
    // Save full progress (local + cloud if logged in)
    static void saveProgress(int coins, int cards, int archerLvl, int sheriffLvl, int allyLvl);
    
    // Account authentication
    static void login(const std::string& email, const std::string& password);
    static void registerAccount(const std::string& email, const std::string& password);
    static void logout();
    static bool isLoggedIn();
    static std::string getCurrentUserEmail();
    static std::string getCurrentUserUid();
    
    // Check if storage is available
    static bool isAvailable();
};

#endif //TOWERDEFENSE_GAMESTORAGE_H
