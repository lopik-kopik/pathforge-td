#ifndef TOWERDEFENSE_HUD_H
#define TOWERDEFENSE_HUD_H

#include "Game.h"

class Shader;
class Model;

// Renders HUD elements using colored quads (no text textures needed)
// Shows: gold amount (yellow bars), wave number (blue bars), base HP (green/red bar), tower cost
class HUD {
public:
    HUD() = default;

    // Draw the HUD overlay
    // mapWidth/mapHeight in world coordinates to position HUD elements
    void render(Shader& shader, const Model& quad, const Game& game,
                float mapWidth, float mapHeight, Model* mageIconModel = nullptr);

    // Draw main menu
    void renderMainMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight, int menuCoins, Model* chestModel = nullptr);
    
    // Handle main menu taps (returns: 0=START, 1=CHEST, 2=CHARACTERS, -1=none)
    int handleMainMenuTap(float worldX, float worldY, float mapWidth, float mapHeight);
    
    // Draw difficulty selection menu
    void renderDifficultyMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight);
    
    // Draw character upgrade menu
    void renderCharacterMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight,
                            int cards, int archerLevel, int sheriffLevel, int allyLevel);
    
    // Handle character menu taps (returns: 0=BACK, 1=UPGRADE_ARCHER, 2=UPGRADE_SHERIFF, 3=UPGRADE_ALLY, 4=CHEST, -1=none)
    int handleCharacterMenuTap(float worldX, float worldY, float mapWidth, float mapHeight);

    // Draw pause menu
    void renderPauseMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight);
    
    // Draw login screen
    void renderLoginScreen(Shader& shader, const Model& quad, float mapWidth, float mapHeight);
    
    // Handle login taps (returns: 0=EMAIL_FIELD, 1=PASSWORD_FIELD, 2=LOGIN_BTN, 3=REGISTER_BTN, 4=SKIP_BTN, -1=none)
    int handleLoginTap(float worldX, float worldY, float mapWidth, float mapHeight);
    
    // Draw sandbox controls
    void renderSandboxControls(Shader& shader, const Model& quad, float mapWidth, float mapHeight, int waveNumber);
    
    // Handle sandbox taps (returns: 0=BACK, 1=CLEAR, 2=NEXT_WAVE, 3=SLIME, 4=GOBLIN, 5=BAT, 6=BOSS, -1=none)
    int handleSandboxTap(float worldX, float worldY, float mapWidth, float mapHeight);

    // Draw upgrade menu for selected tower
    void renderUpgradeMenu(Shader& shader, const Model& quad, const Tower* tower,
                          float mapWidth, float mapHeight, int playerGold);
    
    // Check if tap is on tower selection buttons
    bool handleTowerSelectionTap(float worldX, float worldY, float mapWidth, float mapHeight, Game& game);
    
    // Check if tap is on difficulty selection buttons (returns 0=EASY, 1=MEDIUM, 2=HARD, -1=none)
    int handleDifficultyTap(float worldX, float worldY, float mapWidth, float mapHeight);

private:
    void drawBar(Shader& shader, const Model& quad,
                 float x, float y, float width, float height,
                 float fillRatio,
                 float bgR, float bgG, float bgB,
                 float fgR, float fgG, float fgB);

    void drawRect(Shader& shader, const Model& quad,
                  float x, float y, float width, float height,
                  float r, float g, float b, float a);

    void drawNumber(Shader& shader, const Model& quad,
                    float x, float y, float digitSize, int number,
                    float r, float g, float b);

    void drawDigit(Shader& shader, const Model& quad,
                   float x, float y, float size, int digit,
                   float r, float g, float b);

    void buildModelMatrix(float* outMatrix, float x, float y, float sx, float sy);
};

#endif //TOWERDEFENSE_HUD_H
