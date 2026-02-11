#include "HUD.h"
#include "Shader.h"
#include "Model.h"
#include "Tower.h"
#include "SheriffTower.h"
#include <cstring>
#include <cmath>

void HUD::buildModelMatrix(float* outMatrix, float x, float y, float sx, float sy) {
    // Column-major 4x4: scale then translate
    memset(outMatrix, 0, sizeof(float) * 16);
    outMatrix[0] = sx;   // scaleX
    outMatrix[5] = sy;   // scaleY
    outMatrix[10] = 1.0f;
    outMatrix[12] = x;   // translateX
    outMatrix[13] = y;   // translateY
    outMatrix[15] = 1.0f;
}

void HUD::drawRect(Shader& shader, const Model& quad,
                   float x, float y, float width, float height,
                   float r, float g, float b, float a) {
    float modelMatrix[16];
    // The unit quad goes from -1 to 1, so scale by half-size and position at center
    buildModelMatrix(modelMatrix, x + width * 0.5f, y + height * 0.5f,
                     width * 0.5f, height * 0.5f);
    shader.setModelMatrix(modelMatrix);
    shader.setColor(r, g, b, a);
    shader.drawModel(quad);
}

void HUD::drawBar(Shader& shader, const Model& quad,
                  float x, float y, float width, float height,
                  float fillRatio,
                  float bgR, float bgG, float bgB,
                  float fgR, float fgG, float fgB) {
    // Background
    drawRect(shader, quad, x, y, width, height, bgR, bgG, bgB, 0.8f);

    // Fill
    if (fillRatio > 0.0f) {
        float fillWidth = width * fillRatio;
        drawRect(shader, quad, x, y, fillWidth, height, fgR, fgG, fgB, 1.0f);
    }
}

void HUD::drawDigit(Shader& shader, const Model& quad,
                    float x, float y, float size, int digit,
                    float r, float g, float b) {
    // Draw a digit using simple 3x5 pixel patterns with small rectangles
    // Each segment is a small filled rectangle
    float s = size / 5.0f; // unit size

    // 7-segment-like display using rectangles
    // Segments: top, top-left, top-right, middle, bottom-left, bottom-right, bottom
    //  _
    // |_|
    // |_|

    bool segments[10][7] = {
        // top, topL, topR, mid, botL, botR, bot
        {true,  true,  true,  false, true,  true,  true},  // 0
        {false, false, true,  false, false, true,  false}, // 1
        {true,  false, true,  true,  true,  false, true},  // 2
        {true,  false, true,  true,  false, true,  true},  // 3
        {false, true,  true,  true,  false, true,  false}, // 4
        {true,  true,  false, true,  false, true,  true},  // 5
        {true,  true,  false, true,  true,  true,  true},  // 6
        {true,  false, true,  false, false, true,  false}, // 7
        {true,  true,  true,  true,  true,  true,  true},  // 8
        {true,  true,  true,  true,  false, true,  true},  // 9
    };

    if (digit < 0 || digit > 9) return;

    float thick = s * 0.8f;
    float hSeg = size * 0.5f; // horizontal segment width
    float vSeg = size * 0.35f; // vertical segment height

    // Top horizontal
    if (segments[digit][0])
        drawRect(shader, quad, x + thick, y + size - thick, hSeg, thick, r, g, b, 1.0f);
    // Top-left vertical
    if (segments[digit][1])
        drawRect(shader, quad, x, y + size * 0.5f, thick, vSeg, r, g, b, 1.0f);
    // Top-right vertical
    if (segments[digit][2])
        drawRect(shader, quad, x + thick + hSeg, y + size * 0.5f, thick, vSeg, r, g, b, 1.0f);
    // Middle horizontal
    if (segments[digit][3])
        drawRect(shader, quad, x + thick, y + size * 0.5f - thick * 0.5f, hSeg, thick, r, g, b, 1.0f);
    // Bottom-left vertical
    if (segments[digit][4])
        drawRect(shader, quad, x, y + thick, thick, vSeg, r, g, b, 1.0f);
    // Bottom-right vertical
    if (segments[digit][5])
        drawRect(shader, quad, x + thick + hSeg, y + thick, thick, vSeg, r, g, b, 1.0f);
    // Bottom horizontal
    if (segments[digit][6])
        drawRect(shader, quad, x + thick, y, hSeg, thick, r, g, b, 1.0f);
}

void HUD::drawNumber(Shader& shader, const Model& quad,
                     float x, float y, float digitSize, int number,
                     float r, float g, float b) {
    if (number < 0) number = 0;

    // Convert number to digits
    int digits[10];
    int count = 0;

    if (number == 0) {
        digits[0] = 0;
        count = 1;
    } else {
        int temp = number;
        while (temp > 0 && count < 10) {
            digits[count++] = temp % 10;
            temp /= 10;
        }
    }

    // Draw right to left
    float digitWidth = digitSize * 0.8f;
    float startX = x;
    for (int i = count - 1; i >= 0; i--) {
        drawDigit(shader, quad, startX, y, digitSize, digits[i], r, g, b);
        startX += digitWidth;
    }
}

void HUD::render(Shader& shader, const Model& quad, const Game& game,
                 float mapWidth, float mapHeight, Model* mageIconModel) {
    // HUD positioned at top of screen
    float hudY = mapHeight - 1.4f;
    float margin = 0.15f;
    float barHeight = 0.35f;
    float digitSize = 0.5f;

    // === Base HP bar (top-left) ===
    // Red background, green fill
    float hpRatio = (float)game.getBaseHP() / 20.0f;
    drawBar(shader, quad,
            margin, hudY, 3.0f, barHeight, hpRatio,
            0.5f, 0.1f, 0.1f,  // bg: dark red
            0.1f, 0.8f, 0.1f); // fg: green

    // HP icon (heart-like: small red square)
    drawRect(shader, quad, margin + 0.1f, hudY + barHeight + 0.05f, 0.3f, 0.3f,
             0.9f, 0.2f, 0.2f, 1.0f);

    // HP number
    drawNumber(shader, quad, margin + 0.5f, hudY + barHeight + 0.05f,
               digitSize * 0.6f, game.getBaseHP(), 1.0f, 1.0f, 1.0f);

    // === Gold display (top-center) ===
    // Gold coin icon (yellow square)
    float goldX = 3.8f;
    drawRect(shader, quad, goldX, hudY + barHeight + 0.05f, 0.3f, 0.3f,
             1.0f, 0.85f, 0.0f, 1.0f);

    // Gold number
    drawNumber(shader, quad, goldX + 0.4f, hudY + barHeight + 0.05f,
               digitSize * 0.6f, game.getGold(), 1.0f, 0.9f, 0.0f);

    // === Wave indicator (top-right) ===
    float waveX = 7.0f;
    // Wave icon (blue square)
    drawRect(shader, quad, waveX, hudY + barHeight + 0.05f, 0.3f, 0.3f,
             0.2f, 0.4f, 0.9f, 1.0f);

    // Wave number
    drawNumber(shader, quad, waveX + 0.4f, hudY + barHeight + 0.05f,
               digitSize * 0.6f, game.getDisplayedWave(), 0.8f, 0.8f, 1.0f);

    // === Pause button (top-right corner) ===
    float pauseX = mapWidth - 0.9f;
    float pauseY = mapHeight - 0.9f;
    // Button background
    drawRect(shader, quad, pauseX, pauseY, 0.6f, 0.6f, 0.3f, 0.3f, 0.4f, 0.8f);
    // Pause symbol (two vertical bars)
    drawRect(shader, quad, pauseX + 0.15f, pauseY + 0.1f, 0.12f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, pauseX + 0.33f, pauseY + 0.1f, 0.12f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f);

    // === Tower cost indicator (bottom of screen) ===
    float costY = 0.1f;
    // Background panel
    drawRect(shader, quad, margin, costY, mapWidth - 2 * margin, 1.0f,
             0.15f, 0.15f, 0.25f, 0.7f);

    // === Tower Selection Buttons ===
    float btnY = costY + 0.1f;
    float btnSize = 0.8f;
    float archerX = margin + 0.2f;
    float sheriffX = margin + 2.0f;
    
    // Archer button background (highlighted if selected)
    bool archerSelected = (game.getSelectedTowerType() == 0);
    float archerR = archerSelected ? 0.3f : 0.15f;
    float archerG = archerSelected ? 0.6f : 0.15f;
    float archerB = archerSelected ? 0.3f : 0.25f;
    drawRect(shader, quad, archerX, btnY, btnSize, btnSize, archerR, archerG, archerB, 1.0f);
    // Archer button border
    if (archerSelected) {
        float border = 0.05f;
        drawRect(shader, quad, archerX - border, btnY - border, btnSize + 2*border, border, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, archerX - border, btnY + btnSize, btnSize + 2*border, border, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, archerX - border, btnY, border, btnSize, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, archerX + btnSize, btnY, border, btnSize, 1.0f, 1.0f, 0.0f, 1.0f);
    }
    // Archer cost
    drawNumber(shader, quad, archerX + 0.15f, btnY - 0.4f,
               digitSize * 0.4f, Tower::getCost(game.getDifficulty()), 1.0f, 0.9f, 0.0f);
    
    // Sheriff button background (highlighted if selected)
    bool sheriffSelected = (game.getSelectedTowerType() == 1);
    float sheriffR = sheriffSelected ? 0.6f : 0.15f;
    float sheriffG = sheriffSelected ? 0.5f : 0.15f;
    float sheriffB = sheriffSelected ? 0.3f : 0.25f;
    drawRect(shader, quad, sheriffX, btnY, btnSize, btnSize, sheriffR, sheriffG, sheriffB, 1.0f);
    // Sheriff button border
    if (sheriffSelected) {
        float border = 0.05f;
        drawRect(shader, quad, sheriffX - border, btnY - border, btnSize + 2*border, border, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, sheriffX - border, btnY + btnSize, btnSize + 2*border, border, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, sheriffX - border, btnY, border, btnSize, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, sheriffX + btnSize, btnY, border, btnSize, 1.0f, 1.0f, 0.0f, 1.0f);
    }
    // Sheriff cost
    drawNumber(shader, quad, sheriffX + 0.15f, btnY - 0.4f,
               digitSize * 0.4f, SheriffTower::getCost(game.getDifficulty()), 1.0f, 0.9f, 0.0f);
    
    // === MAGE button (BIGGER) ===
    float mageBtnSize = 1.0f; // Bigger button
    float mageX = margin + 3.8f;
    bool mageSelected = (game.getSelectedTowerType() == 2);
    
    // Draw mage icon texture if available, otherwise fallback to colored rect
    if (mageIconModel) {
        float modelMatrix[16];
        buildModelMatrix(modelMatrix, mageX + mageBtnSize*0.5f, btnY + mageBtnSize*0.5f, 
                         mageBtnSize*0.5f, mageBtnSize*0.5f);
        shader.setModelMatrix(modelMatrix);
        shader.setColor(1.0f, 1.0f, 1.0f, 1.0f);
        shader.setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
        shader.drawModel(*mageIconModel);
    } else {
        float mageR = mageSelected ? 0.8f : 0.6f;
        float mageG = mageSelected ? 0.3f : 0.2f;
        float mageB = mageSelected ? 0.8f : 0.5f;
        drawRect(shader, quad, mageX, btnY, mageBtnSize, mageBtnSize, mageR, mageG, mageB, 1.0f);
    }
    // Mage button border
    if (mageSelected) {
        float border = 0.05f;
        drawRect(shader, quad, mageX - border, btnY - border, mageBtnSize + 2*border, border, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, mageX - border, btnY + mageBtnSize, mageBtnSize + 2*border, border, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, mageX - border, btnY, border, mageBtnSize, 1.0f, 1.0f, 0.0f, 1.0f);
        drawRect(shader, quad, mageX + mageBtnSize, btnY, border, mageBtnSize, 1.0f, 1.0f, 0.0f, 1.0f);
        
        // Element selection buttons (only show when mage is selected) - BIGGER (0.5)
        float elemSize = 0.5f;
        float elemY = btnY + mageBtnSize + 0.15f;
        
        // Fire element
        ElementType currentElem = game.getSelectedMageElement();
        float fireX = mageX;
        drawRect(shader, quad, fireX, elemY, elemSize, elemSize, 
                 currentElem == ElementType::FIRE ? 1.0f : 0.6f, 
                 currentElem == ElementType::FIRE ? 0.3f : 0.1f, 0.0f, 1.0f);
        
        // Ice element
        float iceX = mageX + elemSize + 0.05f;
        drawRect(shader, quad, iceX, elemY, elemSize, elemSize, 
                 currentElem == ElementType::ICE ? 0.3f : 0.1f, 
                 currentElem == ElementType::ICE ? 0.8f : 0.5f, 
                 currentElem == ElementType::ICE ? 1.0f : 0.6f, 1.0f);
        
        // Lightning element
        float lightX = mageX + 2*(elemSize + 0.05f);
        drawRect(shader, quad, lightX, elemY, elemSize, elemSize, 
                 currentElem == ElementType::LIGHTNING ? 1.0f : 0.7f, 
                 currentElem == ElementType::LIGHTNING ? 1.0f : 0.7f, 0.0f, 1.0f);
    }
    // Mage cost
    drawNumber(shader, quad, mageX + 0.15f, btnY - 0.4f,
               digitSize * 0.4f, MageTower::getCost(game.getDifficulty()), 1.0f, 0.9f, 0.0f);

    // === Game Over / Victory overlay ===
    if (game.getState() == GameState::GAME_OVER) {
        // Dark overlay
        drawRect(shader, quad, 0, mapHeight * 0.35f, mapWidth, mapHeight * 0.3f,
                 0.8f, 0.1f, 0.1f, 0.85f);
    } else if (game.getState() == GameState::VICTORY) {
        // Victory overlay
        drawRect(shader, quad, 0, mapHeight * 0.35f, mapWidth, mapHeight * 0.3f,
                 0.1f, 0.7f, 0.2f, 0.85f);
        
        // MENU button on victory screen
        float menuBtnWidth = 3.0f;
        float menuBtnHeight = 1.0f;
        float menuBtnX = mapWidth * 0.5f - menuBtnWidth * 0.5f;
        float menuBtnY = mapHeight * 0.45f;
        // Button background (gold)
        drawRect(shader, quad, menuBtnX, menuBtnY, menuBtnWidth, menuBtnHeight, 
                 0.9f, 0.7f, 0.1f, 1.0f);
        // Button border
        float border = 0.05f;
        drawRect(shader, quad, menuBtnX, menuBtnY, menuBtnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
        drawRect(shader, quad, menuBtnX, menuBtnY + menuBtnHeight - border, menuBtnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
        drawRect(shader, quad, menuBtnX, menuBtnY, border, menuBtnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
        drawRect(shader, quad, menuBtnX + menuBtnWidth - border, menuBtnY, border, menuBtnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    }

}

void HUD::renderMainMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight, int menuCoins, Model* chestModel) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    // Dark background
    drawRect(shader, quad, 0, 0, mapWidth, mapHeight, 0.05f, 0.1f, 0.05f, 0.95f);
    
    // Draw menu coins in top-right corner
    if (menuCoins > 0) {
        float coinBoxX = mapWidth - 2.5f;
        float coinBoxY = mapHeight - 0.8f;
        float digitSize = 0.25f;
        
        // Coin icon (yellow circle approximated with square)
        drawRect(shader, quad, coinBoxX, coinBoxY, 0.4f, 0.4f, 1.0f, 0.85f, 0.0f, 1.0f);
        
        // Coin amount
        drawNumber(shader, quad, coinBoxX + 0.5f, coinBoxY - 0.05f, digitSize, menuCoins, 1.0f, 0.85f, 0.0f);
    }
    
    // Main title box (one square)
    float boxWidth = 8.0f;
    float boxHeight = 3.5f;
    float boxX = centerX - boxWidth * 0.5f;
    float boxY = centerY + 1.0f;
    
    // Box background
    drawRect(shader, quad, boxX, boxY, boxWidth, boxHeight, 0.15f, 0.25f, 0.15f, 0.9f);
    
    // Box border (white)
    float border = 0.1f;
    drawRect(shader, quad, boxX, boxY, boxWidth, border, 1.0f, 1.0f, 1.0f, 1.0f); // top
    drawRect(shader, quad, boxX, boxY + boxHeight - border, boxWidth, border, 1.0f, 1.0f, 1.0f, 1.0f); // bottom
    drawRect(shader, quad, boxX, boxY, border, boxHeight, 1.0f, 1.0f, 1.0f, 1.0f); // left
    drawRect(shader, quad, boxX + boxWidth - border, boxY, border, boxHeight, 1.0f, 1.0f, 1.0f, 1.0f); // right
    
    // "TOWER DEFENSE" text - all gold color
    float letterSize = 0.45f;
    float spacing = 0.55f;
    float startX = centerX - 3.2f; // Center the whole text (shifted right)
    float textY = boxY + boxHeight * 0.5f - letterSize * 0.5f;
    
    // T
    drawRect(shader, quad, startX, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, startX - letterSize*0.25f, textY + letterSize*0.85f, letterSize*0.65f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // O
    float ox = startX + spacing;
    drawRect(shader, quad, ox, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ox + letterSize*0.45f, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ox, textY + letterSize*0.85f, letterSize*0.6f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ox, textY, letterSize*0.6f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // W
    float wx = startX + spacing * 2;
    drawRect(shader, quad, wx, textY, letterSize*0.12f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, wx + letterSize*0.48f, textY, letterSize*0.12f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, wx, textY, letterSize*0.6f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // E
    float ex = startX + spacing * 3;
    drawRect(shader, quad, ex, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex, textY + letterSize*0.85f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex, textY + letterSize*0.425f, letterSize*0.4f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex, textY, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // R
    float rx = startX + spacing * 4;
    drawRect(shader, quad, rx, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, rx, textY + letterSize*0.85f, letterSize*0.45f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, rx + letterSize*0.45f, textY + letterSize*0.5f, letterSize*0.12f, letterSize*0.5f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, rx, textY + letterSize*0.425f, letterSize*0.35f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // Space between words
    
    // D (DEFENSE)
    float dx = startX + spacing * 5.5f;
    drawRect(shader, quad, dx, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, dx + letterSize*0.45f, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, dx, textY + letterSize*0.85f, letterSize*0.6f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, dx, textY, letterSize*0.6f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // E
    float ex2 = startX + spacing * 6.5f;
    drawRect(shader, quad, ex2, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex2, textY + letterSize*0.85f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex2, textY + letterSize*0.425f, letterSize*0.4f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex2, textY, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // F
    float fx = startX + spacing * 7.5f;
    drawRect(shader, quad, fx, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, fx, textY + letterSize*0.85f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, fx, textY + letterSize*0.425f, letterSize*0.4f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // E (second)
    float ex3 = startX + spacing * 8.5f;
    drawRect(shader, quad, ex3, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex3, textY + letterSize*0.85f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex3, textY + letterSize*0.425f, letterSize*0.4f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex3, textY, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // N
    float nx = startX + spacing * 9.5f;
    drawRect(shader, quad, nx, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, nx + letterSize*0.45f, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    for (int i = 0; i < 4; i++) {
        drawRect(shader, quad, nx + i*letterSize*0.1f, textY + letterSize*0.8f - i*letterSize*0.25f, 
                letterSize*0.1f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    }
    
    // S
    float sx = startX + spacing * 10.5f;
    drawRect(shader, quad, sx, textY + letterSize*0.85f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, sx, textY + letterSize*0.425f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, sx, textY, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, sx, textY + letterSize*0.425f, letterSize*0.15f, letterSize*0.5f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, sx + letterSize*0.35f, textY, letterSize*0.15f, letterSize*0.5f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // E (third)
    float ex4 = startX + spacing * 11.5f;
    drawRect(shader, quad, ex4, textY, letterSize*0.15f, letterSize, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex4, textY + letterSize*0.85f, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex4, textY + letterSize*0.425f, letterSize*0.4f, letterSize*0.12f, 1.0f, 0.85f, 0.0f, 1.0f);
    drawRect(shader, quad, ex4, textY, letterSize*0.5f, letterSize*0.15f, 1.0f, 0.85f, 0.0f, 1.0f);
    
    // START button - bigger
    float btnY = centerY - 2.0f;
    float btnWidth = 5.0f;
    float btnHeight = 1.5f;
    float btnX = centerX - btnWidth*0.5f;
    
    // Button background (green)
    drawRect(shader, quad, btnX, btnY - btnHeight*0.5f, 
            btnWidth, btnHeight, 0.2f, 0.7f, 0.2f, 1.0f);
    
    // Button border
    float btnBorder = 0.1f;
    drawRect(shader, quad, btnX, btnY - btnHeight*0.5f, btnWidth, btnBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnX, btnY + btnHeight*0.5f - btnBorder, btnWidth, btnBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnX, btnY - btnHeight*0.5f, btnBorder, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnX + btnWidth - btnBorder, btnY - btnHeight*0.5f, btnBorder, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // "START" text on button - use different variable names
    float btnTextY = btnY - 0.25f;
    float btnLetterSize = 0.5f;
    float btnSpacing = 0.45f;
    float btnStartX = centerX - 1.3f;
    
    // S
    drawRect(shader, quad, btnStartX, btnTextY + btnLetterSize*0.85f, btnLetterSize*0.5f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnStartX, btnTextY + btnLetterSize*0.425f, btnLetterSize*0.5f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnStartX, btnTextY, btnLetterSize*0.5f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnStartX, btnTextY + btnLetterSize*0.425f, btnLetterSize*0.15f, btnLetterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnStartX + btnLetterSize*0.35f, btnTextY, btnLetterSize*0.15f, btnLetterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // T
    float btnTx = btnStartX + btnSpacing;
    drawRect(shader, quad, btnTx, btnTextY, btnLetterSize*0.15f, btnLetterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnTx - btnLetterSize*0.2f, btnTextY + btnLetterSize*0.85f, btnLetterSize*0.55f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // A
    float btnAx = btnStartX + btnSpacing * 2;
    drawRect(shader, quad, btnAx, btnTextY, btnLetterSize*0.15f, btnLetterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnAx + btnLetterSize*0.4f, btnTextY, btnLetterSize*0.15f, btnLetterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnAx, btnTextY + btnLetterSize*0.85f, btnLetterSize*0.55f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnAx, btnTextY + btnLetterSize*0.45f, btnLetterSize*0.4f, btnLetterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // R
    float btnRx = btnStartX + btnSpacing * 3;
    drawRect(shader, quad, btnRx, btnTextY, btnLetterSize*0.15f, btnLetterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnRx, btnTextY + btnLetterSize*0.85f, btnLetterSize*0.45f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnRx + btnLetterSize*0.45f, btnTextY + btnLetterSize*0.5f, btnLetterSize*0.12f, btnLetterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnRx, btnTextY + btnLetterSize*0.425f, btnLetterSize*0.35f, btnLetterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // T
    float btnTx2 = btnStartX + btnSpacing * 4 + 0.1f;
    drawRect(shader, quad, btnTx2, btnTextY, btnLetterSize*0.15f, btnLetterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, btnTx2 - btnLetterSize*0.2f, btnTextY + btnLetterSize*0.85f, btnLetterSize*0.55f, btnLetterSize*0.15f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === CHEST button (bottom left) ===
    float chestSize = 1.2f;
    float chestX = 0.5f;
    float chestY = 0.5f;
    
    if (chestModel) {
        // Draw chest image
        float modelMatrix[16];
        buildModelMatrix(modelMatrix, chestX + chestSize*0.5f, chestY + chestSize*0.5f, chestSize, chestSize);
        shader.setModelMatrix(modelMatrix);
        shader.setColor(1.0f, 1.0f, 1.0f, 1.0f);
        shader.setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
        shader.drawModel(*chestModel);
    } else {
        // Fallback: gold box
        drawRect(shader, quad, chestX, chestY, chestSize, chestSize, 0.8f, 0.6f, 0.2f, 1.0f);
    }
    // Price: 200
    drawNumber(shader, quad, chestX + 0.15f, chestY - 0.4f, 0.2f, 200, 1.0f, 0.85f, 0.0f);
    
    // === CHARACTERS button (bottom right) ===
    float charWidth = 2.5f;
    float charHeight = 1.0f;
    float charX = mapWidth - charWidth - 0.5f;
    float charY = 0.5f;
    // Button background (blue)
    drawRect(shader, quad, charX, charY, charWidth, charHeight, 0.2f, 0.4f, 0.8f, 1.0f);
    // Border
    float charBorder = 0.05f;
    drawRect(shader, quad, charX, charY, charWidth, charBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, charX, charY + charHeight - charBorder, charWidth, charBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, charX, charY, charBorder, charHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, charX + charWidth - charBorder, charY, charBorder, charHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === SANDBOX button (bottom middle) - BIGGER ===
    float sandboxWidth = 3.0f;
    float sandboxHeight = 1.2f;
    float sandboxX = centerX - sandboxWidth * 0.5f;
    float sandboxY = 0.5f;
    // Button background (teal/cyan)
    drawRect(shader, quad, sandboxX, sandboxY, sandboxWidth, sandboxHeight, 0.0f, 0.7f, 0.7f, 1.0f);
    // Border
    float sbBorder = 0.05f;
    drawRect(shader, quad, sandboxX, sandboxY, sandboxWidth, sbBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sandboxX, sandboxY + sandboxHeight - sbBorder, sandboxWidth, sbBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sandboxX, sandboxY, sbBorder, sandboxHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sandboxX + sandboxWidth - sbBorder, sandboxY, sbBorder, sandboxHeight, 1.0f, 1.0f, 1.0f, 1.0f);
}

int HUD::handleMainMenuTap(float worldX, float worldY, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    // START button
    float startBtnWidth = 5.0f;
    float startBtnHeight = 1.5f;
    float startBtnX = centerX - startBtnWidth * 0.5f;
    float startBtnY = centerY - 2.0f;
    
    // Check START button
    if (worldX >= startBtnX && worldX <= startBtnX + startBtnWidth &&
        worldY >= startBtnY - startBtnHeight*0.5f && worldY <= startBtnY + startBtnHeight*0.5f) {
        return 0; // START
    }
    
    // CHEST button (bottom left)
    float chestBtnSize = 1.2f;
    float chestBtnX = 0.5f;
    float chestBtnY = 0.5f;
    if (worldX >= chestBtnX && worldX <= chestBtnX + chestBtnSize &&
        worldY >= chestBtnY && worldY <= chestBtnY + chestBtnSize) {
        return 1; // CHEST
    }
    
    // CHARACTERS button (bottom right)
    float charBtnWidth = 2.5f;
    float charBtnHeight = 1.0f;
    float charBtnX = mapWidth - charBtnWidth - 0.5f;
    float charBtnY = 0.5f;
    if (worldX >= charBtnX && worldX <= charBtnX + charBtnWidth &&
        worldY >= charBtnY && worldY <= charBtnY + charBtnHeight) {
        return 2; // CHARACTERS
    }
    
    // SANDBOX button (bottom middle) - BIGGER
    float sandboxWidth = 3.0f;
    float sandboxHeight = 1.2f;
    float sandboxX = mapWidth * 0.5f - sandboxWidth * 0.5f;
    float sandboxY = 0.5f;
    if (worldX >= sandboxX && worldX <= sandboxX + sandboxWidth &&
        worldY >= sandboxY && worldY <= sandboxY + sandboxHeight) {
        return 3; // SANDBOX
    }
    
    return -1; // No button tapped
}

void HUD::renderPauseMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    // Menu box
    float menuWidth = 5.0f;
    float menuHeight = 5.0f;
    float menuX = centerX - menuWidth * 0.5f;
    float menuY = centerY - menuHeight * 0.5f;
    
    // Semi-transparent background
    drawRect(shader, quad, menuX, menuY, menuWidth, menuHeight, 0.1f, 0.15f, 0.1f, 0.95f);
    
    // Border
    float border = 0.1f;
    drawRect(shader, quad, menuX, menuY, menuWidth, border, 0.8f, 0.8f, 0.8f, 1.0f);
    drawRect(shader, quad, menuX, menuY + menuHeight - border, menuWidth, border, 0.8f, 0.8f, 0.8f, 1.0f);
    drawRect(shader, quad, menuX, menuY, border, menuHeight, 0.8f, 0.8f, 0.8f, 1.0f);
    drawRect(shader, quad, menuX + menuWidth - border, menuY, border, menuHeight, 0.8f, 0.8f, 0.8f, 1.0f);
    
    // Button dimensions
    float btnWidth = 3.5f;
    float btnHeight = 0.9f;
    float btnX = centerX - btnWidth * 0.5f;
    float letterSize = 0.35f;
    
    // === BACK button (top) ===
    float backY = centerY + 1.5f;
    drawRect(shader, quad, btnX, backY - btnHeight*0.5f, btnWidth, btnHeight, 0.2f, 0.6f, 0.2f, 1.0f);
    // BACK text
    float backTextX = centerX - 0.8f;
    float textY = backY - 0.15f;
    // B
    drawRect(shader, quad, backTextX, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backTextX, textY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backTextX, textY + letterSize*0.4f, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backTextX, textY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backTextX + letterSize*0.35f, textY + letterSize*0.5f, letterSize*0.1f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    // A
    float ax = backTextX + letterSize * 0.6f;
    drawRect(shader, quad, ax, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax + letterSize*0.35f, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax, textY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax + letterSize*0.15f, textY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    // C
    float cx = backTextX + letterSize * 1.2f;
    drawRect(shader, quad, cx, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, cx, textY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, cx, textY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // K
    float kx = backTextX + letterSize * 1.7f;
    drawRect(shader, quad, kx, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, kx + letterSize*0.1f, textY + letterSize*0.4f, letterSize*0.3f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, kx + letterSize*0.15f, textY + letterSize*0.7f, letterSize*0.25f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, kx + letterSize*0.15f, textY + letterSize*0.1f, letterSize*0.25f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === RESTART button (middle) ===
    float restartY = centerY;
    drawRect(shader, quad, btnX, restartY - btnHeight*0.5f, btnWidth, btnHeight, 0.6f, 0.5f, 0.2f, 1.0f);
    // RESTART text
    float restartTextX = centerX - 1.1f;
    textY = restartY - 0.15f;
    // R
    drawRect(shader, quad, restartTextX, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, restartTextX, textY + letterSize*0.85f, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, restartTextX + letterSize*0.3f, textY + letterSize*0.4f, letterSize*0.1f, letterSize*0.55f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, restartTextX, textY + letterSize*0.4f, letterSize*0.28f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    // E
    float ex1 = restartTextX + letterSize * 0.55f;
    drawRect(shader, quad, ex1, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex1, textY + letterSize*0.85f, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex1, textY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex1, textY, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // S
    float sx = restartTextX + letterSize * 1.05f;
    drawRect(shader, quad, sx, textY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx, textY + letterSize*0.4f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx, textY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx, textY + letterSize*0.4f, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx + letterSize*0.28f, textY, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    // T
    float tx1 = restartTextX + letterSize * 1.55f;
    drawRect(shader, quad, tx1, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, tx1 - letterSize*0.15f, textY + letterSize*0.85f, letterSize*0.42f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // A
    float ax2 = restartTextX + letterSize * 2.0f;
    drawRect(shader, quad, ax2, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax2 + letterSize*0.35f, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax2, textY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax2 + letterSize*0.15f, textY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    // R
    float rx2 = restartTextX + letterSize * 2.55f;
    drawRect(shader, quad, rx2, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, rx2, textY + letterSize*0.85f, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, rx2 + letterSize*0.3f, textY + letterSize*0.4f, letterSize*0.1f, letterSize*0.55f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, rx2, textY + letterSize*0.4f, letterSize*0.28f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    // T
    float tx2 = restartTextX + letterSize * 3.05f;
    drawRect(shader, quad, tx2, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, tx2 - letterSize*0.15f, textY + letterSize*0.85f, letterSize*0.42f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === MENU button (bottom) ===
    float menuBtnY = centerY - 1.5f;
    drawRect(shader, quad, btnX, menuBtnY - btnHeight*0.5f, btnWidth, btnHeight, 0.6f, 0.2f, 0.2f, 1.0f);
    // MENU text
    float menuTextX = centerX - 0.75f;
    textY = menuBtnY - 0.15f;
    // M
    drawRect(shader, quad, menuTextX, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, menuTextX + letterSize*0.4f, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, menuTextX, textY + letterSize*0.85f, letterSize*0.52f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, menuTextX + letterSize*0.2f, textY + letterSize*0.4f, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    // E
    float ex2 = menuTextX + letterSize * 0.75f;
    drawRect(shader, quad, ex2, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex2, textY + letterSize*0.85f, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex2, textY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex2, textY, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // N
    float nx = menuTextX + letterSize * 1.25f;
    drawRect(shader, quad, nx, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, nx + letterSize*0.35f, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, nx, textY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // U
    float ux = menuTextX + letterSize * 1.85f;
    drawRect(shader, quad, ux, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ux + letterSize*0.35f, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ux, textY, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
}

void HUD::renderUpgradeMenu(Shader& shader, const Model& quad, const Tower* tower,
                           float mapWidth, float mapHeight, int playerGold) {
    if (!tower) return;
    
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    float menuWidth = 6.0f;
    float menuHeight = 4.0f;
    
    // Semi-transparent background
    drawRect(shader, quad, centerX - menuWidth*0.5f, centerY - menuHeight*0.5f, 
            menuWidth, menuHeight, 0.1f, 0.1f, 0.2f, 0.9f);
    
    // Border
    float borderThick = 0.1f;
    drawRect(shader, quad, centerX - menuWidth*0.5f, centerY - menuHeight*0.5f, 
            menuWidth, borderThick, 0.8f, 0.6f, 0.2f, 1.0f); // bottom
    drawRect(shader, quad, centerX - menuWidth*0.5f, centerY + menuHeight*0.5f - borderThick, 
            menuWidth, borderThick, 0.8f, 0.6f, 0.2f, 1.0f); // top
    drawRect(shader, quad, centerX - menuWidth*0.5f, centerY - menuHeight*0.5f, 
            borderThick, menuHeight, 0.8f, 0.6f, 0.2f, 1.0f); // left
    drawRect(shader, quad, centerX + menuWidth*0.5f - borderThick, centerY - menuHeight*0.5f, 
            borderThick, menuHeight, 0.8f, 0.6f, 0.2f, 1.0f); // right
    
    // Level display (top center)
    int level = tower->getLevel();
    drawNumber(shader, quad, centerX - 0.3f, centerY + 1.2f, 0.6f, level, 1.0f, 1.0f, 1.0f);
    
    // Stats display
    float statsY = centerY + 0.3f;
    // Damage
    drawNumber(shader, quad, centerX - 2.0f, statsY, 0.4f, tower->getDamage(), 1.0f, 0.3f, 0.3f);
    // Range (as integer * 10)
    drawNumber(shader, quad, centerX - 0.3f, statsY, 0.4f, (int)(tower->getRange() * 10), 0.3f, 0.3f, 1.0f);
    // Fire rate (as integer * 10)
    drawNumber(shader, quad, centerX + 1.2f, statsY, 0.4f, (int)(tower->getFireRate() * 10), 1.0f, 0.9f, 0.0f);
    
    // Upgrade button (if not max level)
    if (level < Tower::getMaxLevel()) {
        int upgradeCost = tower->getUpgradeCost();
        bool canAfford = playerGold >= upgradeCost;
        
        // Button background
        float btnR = canAfford ? 0.2f : 0.4f;
        float btnG = canAfford ? 0.7f : 0.4f;
        float btnB = canAfford ? 0.2f : 0.4f;
        drawRect(shader, quad, centerX - 1.5f, centerY - 1.2f, 3.0f, 0.8f, btnR, btnG, btnB, 1.0f);
        
        // Cost
        drawNumber(shader, quad, centerX - 0.4f, centerY - 1.0f, 0.5f, upgradeCost, 1.0f, 1.0f, 1.0f);
    } else {
        // MAX LEVEL text (drawn as simple bars)
        drawRect(shader, quad, centerX - 1.5f, centerY - 1.0f, 3.0f, 0.2f, 0.9f, 0.9f, 0.9f, 1.0f);
    }
}



bool HUD::handleTowerSelectionTap(float worldX, float worldY, float mapWidth, float mapHeight, Game& game) {
    float margin = 0.15f;
    float costY = 0.1f;
    float btnSize = 0.8f;
    float archerX = margin + 0.2f;
    float sheriffX = margin + 2.0f;
    float mageX = margin + 3.8f;
    float btnY = costY + 0.1f;
    
    // Check Archer button
    if (worldX >= archerX && worldX <= archerX + btnSize &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        game.selectArcherTower();
        return true;
    }
    
    // Check Sheriff button
    if (worldX >= sheriffX && worldX <= sheriffX + btnSize &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        game.selectSheriffTower();
        return true;
    }
    
    // Check Mage button (BIGGER - 1.0f)
    float mageBtnSize = 1.0f;
    if (worldX >= mageX && worldX <= mageX + mageBtnSize &&
        worldY >= btnY && worldY <= btnY + mageBtnSize) {
        game.selectMageTower();
        return true;
    }
    
    // Check element buttons (only if mage is selected) - BIGGER (0.5)
    if (game.getSelectedTowerType() == 2) {
        float elemSize = 0.5f;
        float elemY = btnY + mageBtnSize + 0.15f;
        
        // Fire element
        float fireX = mageX;
        if (worldX >= fireX && worldX <= fireX + elemSize &&
            worldY >= elemY && worldY <= elemY + elemSize) {
            game.setMageElement(ElementType::FIRE);
            return true;
        }
        
        // Ice element
        float iceX = mageX + elemSize + 0.05f;
        if (worldX >= iceX && worldX <= iceX + elemSize &&
            worldY >= elemY && worldY <= elemY + elemSize) {
            game.setMageElement(ElementType::ICE);
            return true;
        }
        
        // Lightning element
        float lightX = mageX + 2*(elemSize + 0.05f);
        if (worldX >= lightX && worldX <= lightX + elemSize &&
            worldY >= elemY && worldY <= elemY + elemSize) {
            game.setMageElement(ElementType::LIGHTNING);
            return true;
        }
    }
    
    return false; // Not on buttons
}

void HUD::renderDifficultyMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f + 0.5f; // Shift up slightly for 4 buttons
    
    // Dark background
    drawRect(shader, quad, 0, 0, mapWidth, mapHeight, 0.05f, 0.1f, 0.05f, 0.95f);
    
    // Title
    float titleY = centerY + 3.2f;
    // "SELECT DIFFICULTY" - simplified as bars
    drawRect(shader, quad, centerX - 3.5f, titleY, 7.0f, 1.0f, 0.2f, 0.3f, 0.2f, 0.8f);
    
    // Button dimensions
    float btnWidth = 4.0f;
    float btnHeight = 1.0f; // Slightly smaller for 4 buttons
    float border = 0.08f;
    float letterSize = 0.35f;
    
    // === EASY button (green) ===
    float easyY = centerY + 1.8f;
    float easyX = centerX - btnWidth * 0.5f;
    drawRect(shader, quad, easyX, easyY - btnHeight*0.5f, btnWidth, btnHeight, 0.2f, 0.7f, 0.2f, 1.0f);
    // Border
    drawRect(shader, quad, easyX, easyY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, easyX, easyY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, easyX, easyY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, easyX + btnWidth - border, easyY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    // EASY text
    float textY = easyY - 0.15f;
    float textX = centerX - 0.85f;
    // E
    drawRect(shader, quad, textX, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, textX, textY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, textX, textY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, textX, textY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // A
    float ax = textX + letterSize * 0.6f;
    drawRect(shader, quad, ax, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax + letterSize*0.35f, textY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ax, textY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // S
    float sx = textX + letterSize * 1.2f;
    drawRect(shader, quad, sx, textY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx, textY + letterSize*0.4f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx, textY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // Y
    float yx = textX + letterSize * 1.8f;
    drawRect(shader, quad, yx, textY + letterSize*0.4f, letterSize*0.12f, letterSize*0.6f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, yx + letterSize*0.35f, textY + letterSize*0.4f, letterSize*0.12f, letterSize*0.6f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, yx, textY + letterSize*0.4f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === MEDIUM button (yellow/orange) ===
    float medY = centerY + 0.6f;
    float medX = centerX - btnWidth * 0.5f;
    drawRect(shader, quad, medX, medY - btnHeight*0.5f, btnWidth, btnHeight, 0.8f, 0.6f, 0.2f, 1.0f);
    // Border
    drawRect(shader, quad, medX, medY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, medX, medY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, medX, medY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, medX + btnWidth - border, medY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === HARD button (red) ===
    float hardY = centerY - 0.6f;
    float hardX = centerX - btnWidth * 0.5f;
    drawRect(shader, quad, hardX, hardY - btnHeight*0.5f, btnWidth, btnHeight, 0.8f, 0.2f, 0.2f, 1.0f);
    // Border
    drawRect(shader, quad, hardX, hardY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hardX, hardY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hardX, hardY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hardX + btnWidth - border, hardY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    // HARD text
    float hardTextX = centerX - 0.75f;
    float hardTextY = hardY - 0.15f;
    // H
    drawRect(shader, quad, hardTextX, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hardTextX + letterSize*0.35f, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hardTextX, hardTextY + letterSize*0.4f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // A
    float hax = hardTextX + letterSize * 0.6f;
    drawRect(shader, quad, hax, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hax + letterSize*0.35f, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hax, hardTextY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // R
    float hrx = hardTextX + letterSize * 1.2f;
    drawRect(shader, quad, hrx, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hrx, hardTextY + letterSize*0.85f, letterSize*0.35f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hrx + letterSize*0.3f, hardTextY + letterSize*0.4f, letterSize*0.1f, letterSize*0.55f, 1.0f, 1.0f, 1.0f, 1.0f);
    // D
    float hdx = hardTextX + letterSize * 1.8f;
    drawRect(shader, quad, hdx, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hdx + letterSize*0.35f, hardTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hdx, hardTextY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, hdx, hardTextY, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === ENDLESS button (purple) ===
    float endY = centerY - 1.8f;
    float endX = centerX - btnWidth * 0.5f;
    drawRect(shader, quad, endX, endY - btnHeight*0.5f, btnWidth, btnHeight, 0.6f, 0.2f, 0.7f, 1.0f);
    // Border
    drawRect(shader, quad, endX, endY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, endX, endY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, endX, endY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, endX + btnWidth - border, endY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    // ENDLESS text
    float endTextX = centerX - 1.2f;
    float endTextY = endY - 0.15f;
    // E
    drawRect(shader, quad, endTextX, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, endTextX, endTextY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, endTextX, endTextY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, endTextX, endTextY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // N
    float nx = endTextX + letterSize * 0.6f;
    drawRect(shader, quad, nx, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, nx + letterSize*0.35f, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, nx, endTextY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // D
    float edx = endTextX + letterSize * 1.2f;
    drawRect(shader, quad, edx, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, edx + letterSize*0.35f, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, edx, endTextY + letterSize*0.85f, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, edx, endTextY, letterSize*0.47f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // L
    float lx = endTextX + letterSize * 1.85f;
    drawRect(shader, quad, lx, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, lx, endTextY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // E
    float ex2 = endTextX + letterSize * 2.4f;
    drawRect(shader, quad, ex2, endTextY, letterSize*0.12f, letterSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex2, endTextY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex2, endTextY + letterSize*0.4f, letterSize*0.3f, letterSize*0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, ex2, endTextY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    // S
    float sx2 = endTextX + letterSize * 3.0f;
    drawRect(shader, quad, sx2, endTextY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx2, endTextY + letterSize*0.4f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx2, endTextY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx2, endTextY + letterSize*0.4f, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx2 + letterSize*0.28f, endTextY, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    // S
    float sx3 = endTextX + letterSize * 3.6f;
    drawRect(shader, quad, sx3, endTextY + letterSize*0.85f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx3, endTextY + letterSize*0.4f, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx3, endTextY, letterSize*0.4f, letterSize*0.12f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx3, endTextY + letterSize*0.4f, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, sx3 + letterSize*0.28f, endTextY, letterSize*0.12f, letterSize*0.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // Difficulty descriptions (simplified as dark bars)
    // EASY: More gold, weaker enemies
    drawRect(shader, quad, centerX - 2.0f, easyY - 0.75f, 4.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.5f);
    // MEDIUM: Normal
    drawRect(shader, quad, centerX - 2.0f, medY - 0.75f, 4.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.5f);
    // HARD: Less gold, stronger enemies
    drawRect(shader, quad, centerX - 2.0f, hardY - 0.75f, 4.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.5f);
    // ENDLESS: Infinite waves
    drawRect(shader, quad, centerX - 2.0f, endY - 0.75f, 4.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.5f);
}

int HUD::handleDifficultyTap(float worldX, float worldY, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f + 0.5f; // Same offset as in render
    float btnWidth = 4.0f;
    float btnHeight = 1.0f; // Updated to match render
    float easyY = centerY + 1.8f;
    float medY = centerY + 0.6f;
    float hardY = centerY - 0.6f;
    float endY = centerY - 1.8f;
    float btnX = centerX - btnWidth * 0.5f;
    
    // Check EASY button
    if (worldX >= btnX && worldX <= btnX + btnWidth &&
        worldY >= easyY - btnHeight*0.5f && worldY <= easyY + btnHeight*0.5f) {
        return 0; // EASY
    }
    
    // Check MEDIUM button
    if (worldX >= btnX && worldX <= btnX + btnWidth &&
        worldY >= medY - btnHeight*0.5f && worldY <= medY + btnHeight*0.5f) {
        return 1; // MEDIUM
    }
    
    // Check HARD button
    if (worldX >= btnX && worldX <= btnX + btnWidth &&
        worldY >= hardY - btnHeight*0.5f && worldY <= hardY + btnHeight*0.5f) {
        return 2; // HARD
    }
    
    // Check ENDLESS button
    if (worldX >= btnX && worldX <= btnX + btnWidth &&
        worldY >= endY - btnHeight*0.5f && worldY <= endY + btnHeight*0.5f) {
        return 3; // ENDLESS
    }
    
    return -1; // No button tapped
}


void HUD::renderCharacterMenu(Shader& shader, const Model& quad, float mapWidth, float mapHeight,
                              int cards, int archerLevel, int sheriffLevel, int allyLevel) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    // Dark background
    drawRect(shader, quad, 0, 0, mapWidth, mapHeight, 0.05f, 0.1f, 0.05f, 0.95f);
    
    // Title
    float titleY = centerY + 3.5f;
    drawRect(shader, quad, centerX - 3.0f, titleY, 6.0f, 0.8f, 0.2f, 0.3f, 0.2f, 0.8f);
    
    // Cards display (top right)
    float cardBoxX = mapWidth - 2.0f;
    float cardBoxY = mapHeight - 0.8f;
    drawRect(shader, quad, cardBoxX, cardBoxY, 0.35f, 0.35f, 1.0f, 0.5f, 0.0f, 1.0f); // Card icon
    drawNumber(shader, quad, cardBoxX + 0.5f, cardBoxY - 0.05f, 0.25f, cards, 1.0f, 0.85f, 0.0f);
    
    // Character upgrade buttons
    float btnWidth = 4.0f;
    float btnHeight = 1.0f;
    float border = 0.05f;
    float letterSize = 0.3f;
    
    // Archer upgrade button
    float archerY = centerY + 1.5f;
    float archerCost = 10 + (archerLevel - 1) * 20;
    drawRect(shader, quad, centerX - btnWidth*0.5f, archerY - btnHeight*0.5f, btnWidth, btnHeight, 0.3f, 0.6f, 0.3f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, archerY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, archerY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, archerY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX + btnWidth*0.5f - border, archerY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    // Level text
    drawNumber(shader, quad, centerX - 1.5f, archerY - 0.15f, letterSize, archerLevel, 1.0f, 1.0f, 1.0f);
    // Cost
    drawNumber(shader, quad, centerX + 0.5f, archerY - 0.15f, letterSize, (int)archerCost, 1.0f, 0.5f, 0.0f);
    
    // Sheriff upgrade button
    float sheriffY = centerY + 0.0f;
    float sheriffCost = 10 + (sheriffLevel - 1) * 20;
    drawRect(shader, quad, centerX - btnWidth*0.5f, sheriffY - btnHeight*0.5f, btnWidth, btnHeight, 0.6f, 0.5f, 0.3f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, sheriffY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, sheriffY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, sheriffY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX + btnWidth*0.5f - border, sheriffY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawNumber(shader, quad, centerX - 1.5f, sheriffY - 0.15f, letterSize, sheriffLevel, 1.0f, 1.0f, 1.0f);
    drawNumber(shader, quad, centerX + 0.5f, sheriffY - 0.15f, letterSize, (int)sheriffCost, 1.0f, 0.5f, 0.0f);
    
    // Ally upgrade button
    float allyY = centerY - 1.5f;
    float allyCost = 10 + (allyLevel - 1) * 20;
    drawRect(shader, quad, centerX - btnWidth*0.5f, allyY - btnHeight*0.5f, btnWidth, btnHeight, 0.4f, 0.4f, 0.6f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, allyY - btnHeight*0.5f, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, allyY + btnHeight*0.5f - border, btnWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, allyY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX + btnWidth*0.5f - border, allyY - btnHeight*0.5f, border, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawNumber(shader, quad, centerX - 1.5f, allyY - 0.15f, letterSize, allyLevel, 1.0f, 1.0f, 1.0f);
    drawNumber(shader, quad, centerX + 0.5f, allyY - 0.15f, letterSize, (int)allyCost, 1.0f, 0.5f, 0.0f);
    
    // BACK button
    float backY = centerY - 3.0f;
    float backWidth = 2.0f;
    float backHeight = 0.8f;
    drawRect(shader, quad, centerX - backWidth*0.5f, backY - backHeight*0.5f, backWidth, backHeight, 0.5f, 0.2f, 0.2f, 1.0f);
    drawRect(shader, quad, centerX - backWidth*0.5f, backY - backHeight*0.5f, backWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - backWidth*0.5f, backY + backHeight*0.5f - border, backWidth, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - backWidth*0.5f, backY - backHeight*0.5f, border, backHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX + backWidth*0.5f - border, backY - backHeight*0.5f, border, backHeight, 1.0f, 1.0f, 1.0f, 1.0f);
}

int HUD::handleCharacterMenuTap(float worldX, float worldY, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    float btnWidth = 4.0f;
    float btnHeight = 1.0f;
    
    // Archer button
    float archerY = centerY + 1.5f;
    if (worldX >= centerX - btnWidth*0.5f && worldX <= centerX + btnWidth*0.5f &&
        worldY >= archerY - btnHeight*0.5f && worldY <= archerY + btnHeight*0.5f) {
        return 1; // UPGRADE_ARCHER
    }
    
    // Sheriff button
    float sheriffY = centerY + 0.0f;
    if (worldX >= centerX - btnWidth*0.5f && worldX <= centerX + btnWidth*0.5f &&
        worldY >= sheriffY - btnHeight*0.5f && worldY <= sheriffY + btnHeight*0.5f) {
        return 2; // UPGRADE_SHERIFF
    }
    
    // Ally button
    float allyY = centerY - 1.5f;
    if (worldX >= centerX - btnWidth*0.5f && worldX <= centerX + btnWidth*0.5f &&
        worldY >= allyY - btnHeight*0.5f && worldY <= allyY + btnHeight*0.5f) {
        return 3; // UPGRADE_ALLY
    }
    
    // BACK button
    float backY = centerY - 3.0f;
    float backWidth = 2.0f;
    float backHeight = 0.8f;
    if (worldX >= centerX - backWidth*0.5f && worldX <= centerX + backWidth*0.5f &&
        worldY >= backY - backHeight*0.5f && worldY <= backY + backHeight*0.5f) {
        return 0; // BACK
    }
    
    return -1; // No button tapped
}

void HUD::renderLoginScreen(Shader& shader, const Model& quad, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    // Background
    drawRect(shader, quad, 0, 0, mapWidth, mapHeight, 0.05f, 0.1f, 0.15f, 0.95f);
    
    // Title: "TOWER DEFENSE"
    float titleY = mapHeight - 1.5f;
    // Simple title as yellow bar for now
    drawRect(shader, quad, centerX - 3.0f, titleY, 6.0f, 0.8f, 0.9f, 0.8f, 0.1f, 0.9f);
    
    // Login box
    float boxWidth = 6.0f;
    float boxHeight = 7.0f;
    float boxX = centerX - boxWidth * 0.5f;
    float boxY = centerY - boxHeight * 0.5f + 0.5f;
    
    // Box background
    drawRect(shader, quad, boxX, boxY, boxWidth, boxHeight, 0.15f, 0.2f, 0.25f, 0.95f);
    
    // Box border
    float border = 0.05f;
    drawRect(shader, quad, boxX, boxY, boxWidth, border, 0.5f, 0.6f, 0.7f, 1.0f);
    drawRect(shader, quad, boxX, boxY + boxHeight - border, boxWidth, border, 0.5f, 0.6f, 0.7f, 1.0f);
    drawRect(shader, quad, boxX, boxY, border, boxHeight, 0.5f, 0.6f, 0.7f, 1.0f);
    drawRect(shader, quad, boxX + boxWidth - border, boxY, border, boxHeight, 0.5f, 0.6f, 0.7f, 1.0f);
    
    // Email field placeholder
    float fieldWidth = 4.5f;
    float fieldHeight = 0.8f;
    float emailY = boxY + boxHeight - 1.5f;
    drawRect(shader, quad, centerX - fieldWidth*0.5f, emailY, fieldWidth, fieldHeight, 0.3f, 0.35f, 0.4f, 1.0f);
    // Email label
    drawRect(shader, quad, centerX - 1.5f, emailY + fieldHeight + 0.1f, 3.0f, 0.15f, 0.7f, 0.7f, 0.7f, 1.0f);
    
    // Password field placeholder
    float passY = emailY - 1.3f;
    drawRect(shader, quad, centerX - fieldWidth*0.5f, passY, fieldWidth, fieldHeight, 0.3f, 0.35f, 0.4f, 1.0f);
    // Password label
    drawRect(shader, quad, centerX - 1.5f, passY + fieldHeight + 0.1f, 3.0f, 0.15f, 0.7f, 0.7f, 0.7f, 1.0f);
    
    // Login button
    float btnWidth = 3.0f;
    float btnHeight = 0.9f;
    float loginY = passY - 1.2f;
    drawRect(shader, quad, centerX - btnWidth*0.5f, loginY, btnWidth, btnHeight, 0.2f, 0.7f, 0.3f, 1.0f);
    // Button border
    float btnBorder = 0.03f;
    drawRect(shader, quad, centerX - btnWidth*0.5f, loginY, btnWidth, btnBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, loginY + btnHeight - btnBorder, btnWidth, btnBorder, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX - btnWidth*0.5f, loginY, btnBorder, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, centerX + btnWidth*0.5f - btnBorder, loginY, btnBorder, btnHeight, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // Register button (smaller, below login)
    float regY = loginY - 0.8f;
    drawRect(shader, quad, centerX - 1.5f, regY, 3.0f, 0.6f, 0.3f, 0.5f, 0.7f, 1.0f);
    
    // Skip/Continue as Guest button (at bottom)
    float skipY = boxY + 0.5f;
    drawRect(shader, quad, centerX - 2.0f, skipY, 4.0f, 0.6f, 0.5f, 0.5f, 0.5f, 0.8f);
}

int HUD::handleLoginTap(float worldX, float worldY, float mapWidth, float mapHeight) {
    float centerX = mapWidth * 0.5f;
    float centerY = mapHeight * 0.5f;
    
    float boxWidth = 6.0f;
    float boxHeight = 7.0f;
    float boxX = centerX - boxWidth * 0.5f;
    float boxY = centerY - boxHeight * 0.5f + 0.5f;
    
    float fieldWidth = 4.5f;
    float fieldHeight = 0.8f;
    float emailY = boxY + boxHeight - 1.5f;
    float passY = emailY - 1.3f;
    
    // Check Email field
    if (worldX >= centerX - fieldWidth*0.5f && worldX <= centerX + fieldWidth*0.5f &&
        worldY >= emailY && worldY <= emailY + fieldHeight) {
        return 0; // EMAIL_FIELD
    }
    
    // Check Password field
    if (worldX >= centerX - fieldWidth*0.5f && worldX <= centerX + fieldWidth*0.5f &&
        worldY >= passY && worldY <= passY + fieldHeight) {
        return 1; // PASSWORD_FIELD
    }
    
    // Check Login button
    float btnWidth = 3.0f;
    float btnHeight = 0.9f;
    float loginY = passY - 1.2f;
    if (worldX >= centerX - btnWidth*0.5f && worldX <= centerX + btnWidth*0.5f &&
        worldY >= loginY && worldY <= loginY + btnHeight) {
        return 2; // LOGIN_BTN
    }
    
    // Check Register button
    float regY = loginY - 0.8f;
    if (worldX >= centerX - 1.5f && worldX <= centerX + 1.5f &&
        worldY >= regY && worldY <= regY + 0.6f) {
        return 3; // REGISTER_BTN
    }
    
    // Check Skip/Guest button
    float skipY = boxY + 0.5f;
    if (worldX >= centerX - 2.0f && worldX <= centerX + 2.0f &&
        worldY >= skipY && worldY <= skipY + 0.6f) {
        return 4; // SKIP_BTN
    }
    
    return -1;
}

void HUD::renderSandboxControls(Shader& shader, const Model& quad, float mapWidth, float mapHeight, int waveNumber) {
    float margin = 0.15f;
    float btnSize = 0.6f;
    float btnY = mapHeight - btnSize - 0.1f;
    float border = 0.03f;
    
    // Sandbox label
    drawRect(shader, quad, margin, mapHeight - 0.4f, 2.0f, 0.3f, 0.8f, 0.4f, 0.1f, 0.9f);
    
    // Wave number display
    drawNumber(shader, quad, mapWidth * 0.5f - 0.5f, mapHeight - 0.5f, 0.25f, waveNumber, 1.0f, 1.0f, 0.0f);
    
    // === Control buttons (bottom left) ===
    
    // BACK button
    float backX = margin;
    drawRect(shader, quad, backX, btnY, btnSize, btnSize, 0.5f, 0.2f, 0.2f, 1.0f);
    drawRect(shader, quad, backX, btnY, btnSize, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backX, btnY + btnSize - border, btnSize, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backX, btnY, border, btnSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, backX + btnSize - border, btnY, border, btnSize, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // CLEAR button (red)
    float clearX = margin + btnSize + 0.1f;
    drawRect(shader, quad, clearX, btnY, btnSize, btnSize, 0.8f, 0.2f, 0.2f, 1.0f);
    drawRect(shader, quad, clearX, btnY, btnSize, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, clearX, btnY + btnSize - border, btnSize, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, clearX, btnY, border, btnSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, clearX + btnSize - border, btnY, border, btnSize, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // NEXT WAVE button (green)
    float waveX = margin + 2*(btnSize + 0.1f);
    drawRect(shader, quad, waveX, btnY, btnSize * 1.5f, btnSize, 0.2f, 0.7f, 0.2f, 1.0f);
    drawRect(shader, quad, waveX, btnY, btnSize * 1.5f, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, waveX, btnY + btnSize - border, btnSize * 1.5f, border, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, waveX, btnY, border, btnSize, 1.0f, 1.0f, 1.0f, 1.0f);
    drawRect(shader, quad, waveX + btnSize * 1.5f - border, btnY, border, btnSize, 1.0f, 1.0f, 1.0f, 1.0f);
    
    // === Enemy spawn buttons (bottom right) ===
    float spawnX = mapWidth - margin - 4*(btnSize + 0.05f);
    
    // SLIME
    drawRect(shader, quad, spawnX, btnY, btnSize, btnSize, 0.2f, 0.8f, 0.2f, 1.0f);
    
    // GOBLIN
    drawRect(shader, quad, spawnX + btnSize + 0.05f, btnY, btnSize, btnSize, 0.8f, 0.3f, 0.2f, 1.0f);
    
    // BAT
    drawRect(shader, quad, spawnX + 2*(btnSize + 0.05f), btnY, btnSize, btnSize, 0.5f, 0.3f, 0.6f, 1.0f);
    
    // BOSS
    drawRect(shader, quad, spawnX + 3*(btnSize + 0.05f), btnY, btnSize, btnSize, 0.9f, 0.1f, 0.1f, 1.0f);
}

int HUD::handleSandboxTap(float worldX, float worldY, float mapWidth, float mapHeight) {
    float margin = 0.15f;
    float btnSize = 0.6f;
    float btnY = mapHeight - btnSize - 0.1f;
    
    // BACK button
    float backX = margin;
    if (worldX >= backX && worldX <= backX + btnSize &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 0; // BACK
    }
    
    // CLEAR button
    float clearX = margin + btnSize + 0.1f;
    if (worldX >= clearX && worldX <= clearX + btnSize &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 1; // CLEAR
    }
    
    // NEXT WAVE button
    float waveX = margin + 2*(btnSize + 0.1f);
    if (worldX >= waveX && worldX <= waveX + btnSize * 1.5f &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 2; // NEXT_WAVE
    }
    
    // Enemy spawn buttons
    float spawnX = mapWidth - margin - 4*(btnSize + 0.05f);
    
    // SLIME
    if (worldX >= spawnX && worldX <= spawnX + btnSize &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 3; // SLIME
    }
    
    // GOBLIN
    if (worldX >= spawnX + btnSize + 0.05f && worldX <= spawnX + 2*btnSize + 0.05f &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 4; // GOBLIN
    }
    
    // BAT
    if (worldX >= spawnX + 2*(btnSize + 0.05f) && worldX <= spawnX + 3*btnSize + 0.1f &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 5; // BAT
    }
    
    // BOSS
    if (worldX >= spawnX + 3*(btnSize + 0.05f) && worldX <= spawnX + 4*btnSize + 0.15f &&
        worldY >= btnY && worldY <= btnY + btnSize) {
        return 6; // BOSS
    }
    
    return -1; // No button tapped
}
