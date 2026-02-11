#include "DamageNumber.h"
#include "Shader.h"
#include "Model.h"
#include <cmath>

void DamageNumber::update(float dt) {
    lifetime += dt;
    y += velocityY * dt;
    velocityY -= 2.0f * dt; // Gravity effect
}

float DamageNumber::getAlpha() const {
    // Fade out at end
    if (lifetime > maxLifetime * 0.7f) {
        return 1.0f - (lifetime - maxLifetime * 0.7f) / (maxLifetime * 0.3f);
    }
    return 1.0f;
}

float DamageNumber::getScale() const {
    // Pop in at start
    if (lifetime < 0.1f) {
        return 0.5f + lifetime * 5.0f;
    }
    // Critical hits stay bigger
    return isCritical ? 1.2f : 0.8f;
}

void DamageNumberManager::spawn(float x, float y, int damage, bool critical, bool burn) {
    numbers_.emplace_back(x, y, damage, critical, burn);
}

void DamageNumberManager::update(float dt) {
    for (auto& num : numbers_) {
        num.update(dt);
    }
    
    // Remove dead numbers
    numbers_.erase(
        std::remove_if(numbers_.begin(), numbers_.end(),
            [](const DamageNumber& n) { return !n.isAlive(); }),
        numbers_.end()
    );
}

void DamageNumberManager::render(Shader& shader, const Model& quad) {
    for (const auto& num : numbers_) {
        drawNumber(shader, quad, num);
    }
}

void DamageNumberManager::drawNumber(Shader& shader, const Model& quad, const DamageNumber& num) {
    float alpha = num.getAlpha();
    if (alpha <= 0.0f) return;
    
    // Color based on type
    float r, g, b;
    if (num.isCritical) {
        r = 1.0f; g = 0.2f; b = 0.2f; // Red for crits
    } else if (num.isBurn) {
        r = 1.0f; g = 0.5f; b = 0.0f; // Orange for burn
    } else {
        r = 1.0f; g = 1.0f; b = 1.0f; // White for normal
    }
    
    float scale = num.getScale();
    float digitSize = 0.25f * scale;
    float spacing = digitSize * 0.8f;
    
    // Calculate number width for centering
    int value = num.damage;
    int digits = 0;
    int temp = value;
    do {
        digits++;
        temp /= 10;
    } while (temp > 0);
    if (value == 0) digits = 1;
    
    float totalWidth = (digits - 1) * spacing;
    float startX = num.x - totalWidth * 0.5f;
    float y = num.y;
    
    // Draw each digit
    temp = value;
    if (temp == 0) {
        drawDigit(shader, quad, startX, y, digitSize, 0, r, g, b, alpha);
    } else {
        for (int i = digits - 1; i >= 0; i--) {
            int digit = temp % 10;
            temp /= 10;
            drawDigit(shader, quad, startX + i * spacing, y, digitSize, digit, r, g, b, alpha);
        }
    }
}

void DamageNumberManager::drawDigit(Shader& shader, const Model& quad, float x, float y, float size, int digit, float r, float g, float b, float a) {
    float modelMatrix[16];
    float thickness = size * 0.15f;
    float halfSize = size * 0.5f;
    
    // Simple 7-segment display approximation
    // Top
    if (digit != 1 && digit != 4) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = halfSize * 0.8f;
        modelMatrix[5] = thickness;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x;
        modelMatrix[13] = y + halfSize;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
    
    // Upper left
    if (digit != 1 && digit != 2 && digit != 3 && digit != 7) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = thickness;
        modelMatrix[5] = halfSize * 0.8f;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x - halfSize * 0.4f;
        modelMatrix[13] = y + halfSize * 0.5f;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
    
    // Upper right
    if (digit != 5 && digit != 6) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = thickness;
        modelMatrix[5] = halfSize * 0.8f;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x + halfSize * 0.4f;
        modelMatrix[13] = y + halfSize * 0.5f;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
    
    // Middle
    if (digit != 0 && digit != 1 && digit != 7) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = halfSize * 0.8f;
        modelMatrix[5] = thickness;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x;
        modelMatrix[13] = y;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
    
    // Lower left
    if (digit == 0 || digit == 2 || digit == 6 || digit == 8) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = thickness;
        modelMatrix[5] = halfSize * 0.8f;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x - halfSize * 0.4f;
        modelMatrix[13] = y - halfSize * 0.5f;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
    
    // Lower right
    if (digit != 2) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = thickness;
        modelMatrix[5] = halfSize * 0.8f;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x + halfSize * 0.4f;
        modelMatrix[13] = y - halfSize * 0.5f;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
    
    // Bottom
    if (digit != 1 && digit != 4 && digit != 7) {
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = halfSize * 0.8f;
        modelMatrix[5] = thickness;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = x;
        modelMatrix[13] = y - halfSize;
        shader.setModelMatrix(modelMatrix);
        shader.setColor(r, g, b, a);
        shader.drawModel(quad);
    }
}
