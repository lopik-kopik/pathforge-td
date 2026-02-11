#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <cmath>
#include <cstring>

#include "AndroidOut.h"
#include "Shader.h"
#include "Utility.h"
#include "TextureAsset.h"

//! executes glGetString and outputs the result to logcat
#define PRINT_GL_STRING(s) {aout << #s": "<< glGetString(s) << std::endl;}

#define PRINT_GL_STRING_AS_LIST(s) { \
std::istringstream extensionStream((const char *) glGetString(s));\
std::vector<std::string> extensionList(\
        std::istream_iterator<std::string>{extensionStream},\
        std::istream_iterator<std::string>());\
aout << #s":\n";\
for (auto& extension: extensionList) {\
    aout << extension << "\n";\
}\
aout << std::endl;\
}

// Vertex shader with per-model transform and UV animation
static const char *vertex = R"vertex(#version 300 es
in vec3 inPosition;
in vec2 inUV;

out vec2 fragUV;

uniform mat4 uProjection;
uniform mat4 uModel;
uniform vec4 uUVTransform; // x=offsetU, y=offsetV, z=scaleU, w=scaleV

void main() {
    fragUV = vec2(inUV.x * uUVTransform.z + uUVTransform.x, 
                  inUV.y * uUVTransform.w + uUVTransform.y);
    gl_Position = uProjection * uModel * vec4(inPosition, 1.0);
}
)vertex";

// Fragment shader with color tint
static const char *fragment = R"fragment(#version 300 es
precision mediump float;

in vec2 fragUV;

uniform sampler2D uTexture;
uniform vec4 uColor;

out vec4 outColor;

void main() {
    outColor = texture(uTexture, fragUV) * uColor;
}
)fragment";

static constexpr float kProjectionNearPlane = -1.f;
static constexpr float kProjectionFarPlane = 1.f;

// Map dimensions in world coordinates
static constexpr float MAP_WIDTH = 10.0f;
static constexpr float MAP_HEIGHT = 16.0f;

Renderer::~Renderer() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

void Renderer::buildModelMatrix(float *outMatrix, float x, float y, float sx, float sy) {
    // Column-major 4x4: scale + translate
    memset(outMatrix, 0, sizeof(float) * 16);
    outMatrix[0] = sx;
    outMatrix[5] = sy;
    outMatrix[10] = 1.0f;
    outMatrix[12] = x;
    outMatrix[13] = y;
    outMatrix[15] = 1.0f;
}

void Renderer::screenToWorld(float screenX, float screenY, float &worldX, float &worldY) {
    // Screen coords: (0,0) = top-left, (width_, height_) = bottom-right
    // NDC: (-1,-1) to (1,1)
    // World: based on our projection

    // Normalize to 0..1
    float nx = screenX / (float)width_;
    float ny = screenY / (float)height_;

    // Map to world coordinates
    // Our projection maps [0, MAP_WIDTH] x [0, MAP_HEIGHT] to screen
    // Screen Y is flipped (0 = top, height = bottom) but our world Y=0 is bottom
    worldX = nx * projHalfWidth_ * 2.0f + (projCenterX_ - projHalfWidth_);
    worldY = (1.0f - ny) * projHalfHeight_ * 2.0f + (projCenterY_ - projHalfHeight_);
}

void Renderer::render() {
    updateRenderArea();

    if (shaderNeedsNewProjectionMatrix_) {
        // Build an orthographic projection that maps the grid to screen
        // We want [0, MAP_WIDTH] horizontally and [0, MAP_HEIGHT] vertically to be visible
        // with possible letterboxing

        float aspect = (float)width_ / (float)height_;
        float mapAspect = MAP_WIDTH / MAP_HEIGHT;

        if (aspect > mapAspect) {
            // Screen is wider than map - fit height, extend width
            projHalfHeight_ = MAP_HEIGHT / 2.0f;
            projHalfWidth_ = projHalfHeight_ * aspect;
        } else {
            // Screen is taller than map - fit width, extend height
            projHalfWidth_ = MAP_WIDTH / 2.0f;
            projHalfHeight_ = projHalfWidth_ / aspect;
        }

        projCenterX_ = MAP_WIDTH / 2.0f;
        projCenterY_ = MAP_HEIGHT / 2.0f;

        // Build custom projection matrix centered on the map
        // Maps [centerX - halfW, centerX + halfW] x [centerY - halfH, centerY + halfH] to NDC
        float left = projCenterX_ - projHalfWidth_;
        float right = projCenterX_ + projHalfWidth_;
        float bottom = projCenterY_ - projHalfHeight_;
        float top = projCenterY_ + projHalfHeight_;
        float nearP = kProjectionNearPlane;
        float farP = kProjectionFarPlane;

        float projectionMatrix[16];
        memset(projectionMatrix, 0, sizeof(projectionMatrix));
        // Column-major orthographic projection
        projectionMatrix[0] = 2.0f / (right - left);
        projectionMatrix[5] = 2.0f / (top - bottom);
        projectionMatrix[10] = -2.0f / (farP - nearP);
        projectionMatrix[12] = -(right + left) / (right - left);
        projectionMatrix[13] = -(top + bottom) / (top - bottom);
        projectionMatrix[14] = -(farP + nearP) / (farP - nearP);
        projectionMatrix[15] = 1.0f;

        shader_->setProjectionMatrix(projectionMatrix);
        shaderNeedsNewProjectionMatrix_ = false;
    }

    // Calculate delta time
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - lastFrameTime_).count();
    lastFrameTime_ = now;
    if (dt > 0.1f) dt = 0.1f;

    // Update game logic
    if (game_) {
        game_->update(dt);
    }

    if (game_ && (game_->getState() == GameState::MENU || 
                   game_->getState() == GameState::DIFFICULTY_SELECT ||
                   game_->getState() == GameState::CHARACTER_MENU ||
                   game_->getState() == GameState::SANDBOX)) {
        // Menu background or sandbox
        if (game_->getState() == GameState::SANDBOX) {
            // Sandbox uses game background
            glClearColor(0.15f, 0.25f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            if (game_ && initialized_) {
                renderMap();
                renderTowers();
                renderSheriffTowers();
                renderMageTowers();
                renderAllies();
                renderEnemies();
                renderProjectiles();
                renderHUD();
            }
        } else {
            // Menu background
            glClearColor(0.05f, 0.1f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            renderHUD();
        }
    } else {
        // Dark green background for game
        glClearColor(0.15f, 0.25f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (game_ && initialized_) {
            renderMap();
            renderTowers();
            renderSheriffTowers();
            renderMageTowers();
            renderAllies();
            renderEnemies();
            renderProjectiles();
            // Render particles and damage numbers on top
            game_->getParticles().render(*shader_, *quadWhite_);
            game_->getDamageNumbers().render(*shader_, *quadWhite_);
            renderHUD();
        }
    }

    auto swapResult = eglSwapBuffers(display_, surface_);
    assert(swapResult == EGL_TRUE);
}

void Renderer::renderMap() {
    const Map& map = game_->getMap();
    float modelMatrix[16];

    for (int col = 0; col < Map::COLS; col++) {
        for (int row = 0; row < Map::ROWS; row++) {
            CellType cell = map.getCell(col, row);

            Model* quad = nullptr;
            switch (cell) {
                case CellType::GRASS:
                    quad = quadGrass_;
                    break;
                case CellType::PATH:
                    quad = quadPath_;
                    break;
                case CellType::BLOCKED:
                    quad = quadBlocked_;
                    break;
                case CellType::TOWER:
                    quad = quadGrass_; // draw grass under tower
                    break;
            }

            if (quad) {
                // Cell center position: (col + 0.5, row + 0.5), half-size = 0.5
                buildModelMatrix(modelMatrix, col + 0.5f, row + 0.5f, 0.5f, 0.5f);
                shader_->setModelMatrix(modelMatrix);
                shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
                shader_->drawModel(*quad);
            }
        }
    }
}

void Renderer::renderTowers() {
    float modelMatrix[16];

    // Update tower idle animations (4 frames, 0.2s per frame)
    towerAnimTimer_ += 0.016f;
    int idleFrame = (int)(towerAnimTimer_ / 0.2f) % towerL1Sheet_.getTotalFrames();
    SpriteFrame idleUV = towerL1Sheet_.getFrame(idleFrame);

    for (const auto& tower : game_->getTowers()) {
        // Select texture and spritesheet based on tower level
        Model* towerModel = nullptr;
        Model* attackModel = nullptr;
        SpriteSheet* idleSheet = nullptr;
        SpriteSheet* attackSheet = nullptr;
        
        int level = tower.getLevel();
        if (level >= 3) {
            towerModel = quadTowerL3_;
            attackModel = quadTowerL3Attack_;
            idleSheet = &towerL3Sheet_;
            attackSheet = &towerL3AttackSheet_;
        } else if (level == 2) {
            towerModel = quadTowerL2_;
            attackModel = quadTowerL2Attack_;
            idleSheet = &towerL2Sheet_;
            attackSheet = &towerL2AttackSheet_;
        } else {
            towerModel = quadTowerL1_;
            attackModel = quadTowerL1Attack_;
            idleSheet = &towerL1Sheet_;
            attackSheet = &towerL1AttackSheet_;
        }
        
        buildModelMatrix(modelMatrix, tower.getX(), tower.getY(), 0.675f, 0.675f);
        shader_->setModelMatrix(modelMatrix);
        shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
        
        if (tower.isAttacking()) {
            // Attack animation (6 frames over 0.3s)
            float attackProgress = tower.getAttackAnimProgress();
            int attackFrame = (int)(attackProgress * 6);
            if (attackFrame > 5) attackFrame = 5;
            SpriteFrame attackUV = attackSheet->getFrame(attackFrame);
            shader_->setUVTransform(attackUV.u0, attackUV.v0, 
                                   attackUV.u1 - attackUV.u0, attackUV.v1 - attackUV.v0);
            shader_->drawModel(*attackModel);
        } else {
            // Idle animation
            SpriteFrame idleFrameUV = idleSheet->getFrame(idleFrame % idleSheet->getTotalFrames());
            shader_->setUVTransform(idleFrameUV.u0, idleFrameUV.v0, 
                                   idleFrameUV.u1 - idleFrameUV.u0, idleFrameUV.v1 - idleFrameUV.v0);
            shader_->drawModel(*towerModel);
        }
    }
    
    // Reset UV transform for other render calls
    shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
}

void Renderer::renderEnemies() {
    float modelMatrix[16];

    // Update animation timers (6 frames, 0.15s per frame)
    slimeAnimTimer_ += 0.016f;
    goblinAnimTimer_ += 0.016f;
    int slimeFrame = (int)(slimeAnimTimer_ / 0.15f) % slimeSheet_.getTotalFrames();
    int goblinFrame = (int)(goblinAnimTimer_ / 0.15f) % goblinSheet_.getTotalFrames();

    for (const auto& enemy : game_->getEnemies()) {
        if (enemy.isDead()) continue;

        Model* quad = nullptr;
        SpriteFrame uvFrame{0.0f, 0.0f, 1.0f, 1.0f};

        if (enemy.isDying()) {
            // Death animation
            int deathFrame = enemy.getDeathFrame();
            switch (enemy.getType()) {
                case EnemyType::SLIME:
                    quad = quadSlimeDeath_;
                    deathFrame = deathFrame % slimeDeathSheet_.getTotalFrames();
                    uvFrame = slimeDeathSheet_.getFrame(deathFrame);
                    break;
                case EnemyType::GOBLIN:
                    quad = quadGoblinDeath_;
                    deathFrame = deathFrame % goblinDeathSheet_.getTotalFrames();
                    uvFrame = goblinDeathSheet_.getFrame(deathFrame);
                    break;
                case EnemyType::BOSS:
                    quad = quadGoblinDeath_; // Use goblin death texture for boss
                    deathFrame = deathFrame % goblinDeathSheet_.getTotalFrames();
                    uvFrame = goblinDeathSheet_.getFrame(deathFrame);
                    break;
                case EnemyType::BAT:
                    quad = quadBat_;
                    uvFrame = SpriteFrame{0.0f, 0.0f, 1.0f, 1.0f};
                    break;
                case EnemyType::FLYING_EYE:
                    quad = quadFlyingEye_;
                    uvFrame = SpriteFrame{0.0f, 0.0f, 1.0f, 1.0f};
                    break;
            }
        } else if (enemy.isAlive()) {
            // Normal walk animation
            switch (enemy.getType()) {
                case EnemyType::SLIME:
                    quad = quadSlime_;
                    uvFrame = slimeSheet_.getFrame(slimeFrame);
                    break;
                case EnemyType::GOBLIN:
                    quad = quadGoblin_;
                    uvFrame = goblinSheet_.getFrame(goblinFrame);
                    break;
                case EnemyType::BOSS:
                    quad = quadGoblin_; // Use goblin texture for boss
                    uvFrame = goblinSheet_.getFrame(goblinFrame);
                    break;
                case EnemyType::BAT:
                    quad = quadBat_;
                    uvFrame = SpriteFrame{0.0f, 0.0f, 1.0f, 1.0f};
                    break;
                case EnemyType::FLYING_EYE:
                    quad = quadFlyingEye_;
                    uvFrame = SpriteFrame{0.0f, 0.0f, 1.0f, 1.0f};
                    break;
            }
        }

        if (quad) {
            // Draw enemy sprite with animation
            float enemySize = (enemy.getType() == EnemyType::BOSS) ? 0.9f : 0.6f;
            buildModelMatrix(modelMatrix, enemy.getX(), enemy.getY(), enemySize, enemySize);
            shader_->setModelMatrix(modelMatrix);
            shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
            shader_->setUVTransform(uvFrame.u0, uvFrame.v0,
                                   uvFrame.u1 - uvFrame.u0, uvFrame.v1 - uvFrame.v0);
            shader_->drawModel(*quad);
            
            // Reset UV transform
            shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
            
            // Don't draw HP bar for dying enemies
            if (enemy.isDying()) continue;
            
            // Adjust HP bar position for larger enemies
            float hpBarY = enemy.getY() + 0.65f;

            // Draw HP bar background (red)
            float hpBarWidth = (enemy.getType() == EnemyType::BOSS) ? 1.6f : 0.8f;
            float hpBarHeight = 0.08f;

            buildModelMatrix(modelMatrix,
                             enemy.getX(), hpBarY,
                             hpBarWidth * 0.5f, hpBarHeight * 0.5f);
            shader_->setModelMatrix(modelMatrix);
            shader_->setColor(0.8f, 0.1f, 0.1f, 1.0f);
            shader_->drawModel(*quadWhite_);

            // Draw HP bar fill (green)
            float fillRatio = enemy.getHPRatio();
            if (fillRatio > 0.0f) {
                float fillWidth = hpBarWidth * fillRatio;
                float fillOffset = (hpBarWidth - fillWidth) * 0.5f;
                buildModelMatrix(modelMatrix,
                                 enemy.getX() - fillOffset, hpBarY,
                                 fillWidth * 0.5f, hpBarHeight * 0.5f);
                shader_->setModelMatrix(modelMatrix);
                shader_->setColor(0.1f, 0.9f, 0.1f, 1.0f);
                shader_->drawModel(*quadWhite_);
            }
        }
    }
}

void Renderer::renderSheriffTowers() {
    float modelMatrix[16];

    for (const auto& sheriff : game_->getSheriffTowers()) {
        // Sheriff tower size reduced by 2x (was 0.675, now 0.34)
        buildModelMatrix(modelMatrix, sheriff.getX(), sheriff.getY(), 0.34f, 0.34f);
        shader_->setModelMatrix(modelMatrix);
        shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
        shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
        shader_->drawModel(*quadSheriffTower_);
    }
}

void Renderer::renderMageTowers() {
    float modelMatrix[16];

    for (const auto& mage : game_->getMageTowers()) {
        Model* quad = nullptr;
        float size = 0.4f; // Default size
        
        switch (mage.getElement()) {
            case ElementType::FIRE:
                quad = quadMageFire_;
                size = 0.8f;
                break;
            case ElementType::ICE:
                quad = quadMageIce_;
                size = 1.3f;
                break;
            case ElementType::LIGHTNING:
                quad = quadMageLightning_;
                size = 1.4f;
                break;
        }
        
        if (quad) {
            buildModelMatrix(modelMatrix, mage.getX(), mage.getY(), size, size);
            shader_->setModelMatrix(modelMatrix);
            shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
            shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
            shader_->drawModel(*quad);
        }
    }
}

void Renderer::renderAllies() {
    float modelMatrix[16];

    for (const auto& ally : game_->getAllies()) {
        if (!ally.isAlive()) continue;
        
        // Ally size reduced by 2x (was 0.7, now 0.35)
        buildModelMatrix(modelMatrix, ally.getX(), ally.getY(), 0.35f, 0.35f);
        shader_->setModelMatrix(modelMatrix);
        shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
        shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
        shader_->drawModel(*quadSwordsman_);
        
        // Draw HP bar for ally
        float hpBarY = ally.getY() + 0.65f;
        float hpBarWidth = 0.6f;
        float hpBarHeight = 0.08f;

        // Background (red)
        buildModelMatrix(modelMatrix,
                         ally.getX(), hpBarY,
                         hpBarWidth * 0.5f, hpBarHeight * 0.5f);
        shader_->setModelMatrix(modelMatrix);
        shader_->setColor(0.8f, 0.1f, 0.1f, 1.0f);
        shader_->drawModel(*quadWhite_);

        // Fill (green) based on HP
        float fillRatio = (float)ally.getHP() / (float)ally.getMaxHP();
        if (fillRatio > 0.0f) {
            float fillWidth = hpBarWidth * fillRatio;
            float fillOffset = (hpBarWidth - fillWidth) * 0.5f;
            buildModelMatrix(modelMatrix,
                             ally.getX() - fillOffset, hpBarY,
                             fillWidth * 0.5f, hpBarHeight * 0.5f);
            shader_->setModelMatrix(modelMatrix);
            shader_->setColor(0.1f, 0.9f, 0.1f, 1.0f);
            shader_->drawModel(*quadWhite_);
        }
    }
}

void Renderer::renderProjectiles() {
    float modelMatrix[16];

    for (const auto& proj : game_->getProjectiles()) {
        if (!proj.isActive()) continue;

        buildModelMatrix(modelMatrix, proj.getX(), proj.getY(), 0.1f, 0.1f);
        shader_->setModelMatrix(modelMatrix);
        shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
        shader_->drawModel(*quadProjectile_);
    }
}

void Renderer::renderHUD() {
    if (!quadWhite_) return;
    
    // Render login screen if not logged in
    if (game_->getState() == GameState::LOGIN) {
        hud_.renderLoginScreen(*shader_, *quadWhite_, MAP_WIDTH, MAP_HEIGHT);
        return;
    }
    
    // Render main menu if in menu state
    if (game_->getState() == GameState::MENU) {
        hud_.renderMainMenu(*shader_, *quadWhite_, MAP_WIDTH, MAP_HEIGHT, game_->getMenuCoins(), quadMenuChest_);
        return;
    }
    
    // Render difficulty selection menu
    if (game_->getState() == GameState::DIFFICULTY_SELECT) {
        hud_.renderDifficultyMenu(*shader_, *quadWhite_, MAP_WIDTH, MAP_HEIGHT);
        return;
    }
    
    // Render character menu
    if (game_->getState() == GameState::CHARACTER_MENU) {
        hud_.renderCharacterMenu(*shader_, *quadWhite_, MAP_WIDTH, MAP_HEIGHT,
                                 game_->getCards(), game_->getArcherLevel(),
                                 game_->getSheriffLevel(), game_->getAllyLevel());
        return;
    }
    
    // Render sandbox controls
    if (game_->getState() == GameState::SANDBOX) {
        hud_.render(*shader_, *quadWhite_, *game_, MAP_WIDTH, MAP_HEIGHT, quadMageIcon_);
        hud_.renderSandboxControls(*shader_, *quadWhite_, MAP_WIDTH, MAP_HEIGHT, game_->getSandboxWave());
        return;
    }
    
    hud_.render(*shader_, *quadWhite_, *game_, MAP_WIDTH, MAP_HEIGHT, quadMageIcon_);
    
    // Render tower selection icons (sprites instead of brown squares)
    float margin = 0.15f;
    float costY = 0.1f;
    float btnSize = 0.8f;
    float archerX = margin + 0.2f + btnSize * 0.5f;
    float sheriffX = margin + 2.0f + btnSize * 0.5f;
    float btnY = costY + 0.1f + btnSize * 0.5f;
    float modelMatrix[16];
    
    // Archer icon (level 1 tower attack sprite with 6 frames)
    buildModelMatrix(modelMatrix, archerX, btnY, 0.35f, 0.35f);
    shader_->setModelMatrix(modelMatrix);
    shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    // Use attack sprite (6 frames) instead of idle (4 frames)
    towerAnimTimer_ += 0.016f;
    int uiFrame = (int)(towerAnimTimer_ / 0.2f) % 6; // 6 frames
    SpriteFrame uiUV = towerL1AttackSheet_.getFrame(uiFrame);
    shader_->setUVTransform(uiUV.u0, uiUV.v0, uiUV.u1 - uiUV.u0, uiUV.v1 - uiUV.v0);
    shader_->drawModel(*quadTowerL1Attack_);
    
    // Sheriff icon (sheriff sprite)
    buildModelMatrix(modelMatrix, sheriffX, btnY, 0.35f, 0.35f);
    shader_->setModelMatrix(modelMatrix);
    shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
    shader_->drawModel(*quadSheriff_);
    
    // Reset UV transform
    shader_->setUVTransform(0.0f, 0.0f, 1.0f, 1.0f);
    
    // Render pause menu if paused
    if (game_->isPaused()) {
        hud_.renderPauseMenu(*shader_, *quadWhite_, MAP_WIDTH, MAP_HEIGHT);
    }
    
    // Render upgrade menu if open
    if (game_->isUpgradeMenuOpen()) {
        const Tower* selectedTower = game_->getSelectedTower();
        if (selectedTower) {
            hud_.renderUpgradeMenu(*shader_, *quadWhite_, selectedTower, 
                                  MAP_WIDTH, MAP_HEIGHT, game_->getGold());
        }
    }
}

void Renderer::initGameAssets() {
    auto assetManager = app_->activity->assetManager;

    // Create unit quad vertices (from -1 to 1, UV 0 to 1)
    quadVertices_ = {
        Vertex(Vector3{-1, -1, 0}, Vector2{0, 1}), // bottom-left
        Vertex(Vector3{ 1, -1, 0}, Vector2{1, 1}), // bottom-right
        Vertex(Vector3{ 1,  1, 0}, Vector2{1, 0}), // top-right
        Vertex(Vector3{-1,  1, 0}, Vector2{0, 0}), // top-left
    };
    quadIndices_ = {0, 1, 2, 0, 2, 3};

    // White texture for colored quads (always available)
    texWhite_ = TextureAsset::createSolidColor(255, 255, 255, 255);

    // Load textures from assets, use colored fallbacks if missing
    texGrass_ = TextureAsset::loadAsset(assetManager, "tile_grass.png");
    if (!texGrass_) texGrass_ = TextureAsset::createSolidColor(60, 120, 40, 255); // dark green
    // Force dark green grass
    texGrass_ = TextureAsset::createSolidColor(60, 120, 40, 255);

    texPath_ = TextureAsset::loadAsset(assetManager, "tile_path.png");
    if (!texPath_) texPath_ = TextureAsset::createSolidColor(180, 150, 100, 255); // sandy brown

    texBlocked_ = TextureAsset::loadAsset(assetManager, "tile_tree.png");
    if (!texBlocked_) texBlocked_ = TextureAsset::createSolidColor(34, 100, 34, 255); // dark green

    // Load tower textures by level
    texTowerL1_ = TextureAsset::loadAsset(assetManager, "tower_archer.png");
    if (!texTowerL1_) texTowerL1_ = TextureAsset::createSolidColor(139, 90, 43, 255);
    texTowerL1Attack_ = TextureAsset::loadAsset(assetManager, "tower_archer_1_attack.png");
    if (!texTowerL1Attack_) texTowerL1Attack_ = texTowerL1_;

    texTowerL2_ = TextureAsset::loadAsset(assetManager, "tower_archer_2.png");
    if (!texTowerL2_) texTowerL2_ = texTowerL1_;
    texTowerL2Attack_ = TextureAsset::loadAsset(assetManager, "tower_archer_2_attack.png");
    if (!texTowerL2Attack_) texTowerL2Attack_ = texTowerL2_;

    texTowerL3_ = TextureAsset::loadAsset(assetManager, "tower_archer_3.png");
    if (!texTowerL3_) texTowerL3_ = texTowerL1_;
    texTowerL3Attack_ = TextureAsset::loadAsset(assetManager, "tower_archer_3_attack.png");
    if (!texTowerL3Attack_) texTowerL3Attack_ = texTowerL3_;

    texSlime_ = TextureAsset::loadAsset(assetManager, "enemy_slime.png");
    if (!texSlime_) texSlime_ = TextureAsset::createSolidColor(50, 205, 50, 255); // lime green

    texSlimeDeath_ = TextureAsset::loadAsset(assetManager, "enemy_slime_death.png");
    if (!texSlimeDeath_) texSlimeDeath_ = texSlime_; // fallback

    texGoblin_ = TextureAsset::loadAsset(assetManager, "enemy_goblin.png");
    if (!texGoblin_) texGoblin_ = TextureAsset::createSolidColor(178, 34, 34, 255); // firebrick

    texGoblinDeath_ = TextureAsset::loadAsset(assetManager, "enemy_goblin_death.png");
    if (!texGoblinDeath_) texGoblinDeath_ = texGoblin_; // fallback

    texSwordsman_ = TextureAsset::loadAsset(assetManager, "enemy_swordsman.png");
    if (!texSwordsman_) texSwordsman_ = TextureAsset::createSolidColor(100, 120, 60, 255); // olive

    texSheriff_ = TextureAsset::loadAsset(assetManager, "enemy_sheriff.png");
    if (!texSheriff_) texSheriff_ = TextureAsset::createSolidColor(139, 90, 43, 255); // brown

    texSwordsman_ = TextureAsset::loadAsset(assetManager, "enemy_swordsman.png");
    if (!texSwordsman_) texSwordsman_ = TextureAsset::createSolidColor(100, 120, 60, 255); // olive

    texProjectile_ = TextureAsset::loadAsset(assetManager, "projectile_arrow.png");
    if (!texProjectile_) texProjectile_ = TextureAsset::createSolidColor(255, 255, 100, 255); // yellow

    // Menu chest image
    texMenuChest_ = TextureAsset::loadAsset(assetManager, "Gemini_Generated_Image_jdgjfkjdgjfkjdgj-removebg-preview.png");
    
    // Mage towers - idle animations (GIFs loaded as static for now)
    texMageFire_ = TextureAsset::loadAsset(assetManager, "wizard_animation_157px.gif");
    if (!texMageFire_) texMageFire_ = TextureAsset::loadAsset(assetManager, "fire_wizard_attack.png");
    if (!texMageFire_) texMageFire_ = TextureAsset::createSolidColor(255, 80, 0, 255);
    
    texMageIce_ = TextureAsset::loadAsset(assetManager, "ice_wizard_animation.gif");
    if (!texMageIce_) texMageIce_ = TextureAsset::loadAsset(assetManager, "ice_wizard_attack.png");
    if (!texMageIce_) texMageIce_ = TextureAsset::createSolidColor(100, 200, 255, 255);
    
    texMageLightning_ = TextureAsset::loadAsset(assetManager, "lightning_wizard_v2_aligned.gif");
    if (!texMageLightning_) texMageLightning_ = TextureAsset::loadAsset(assetManager, "lightning_wizard_atack.png");
    if (!texMageLightning_) texMageLightning_ = TextureAsset::createSolidColor(255, 255, 0, 255);
    
    // Mage attack animations (separate)
    texMageFireAttack_ = TextureAsset::loadAsset(assetManager, "fire_wizard_attack.png");
    if (!texMageFireAttack_) texMageFireAttack_ = texMageFire_;
    
    texMageIceAttack_ = TextureAsset::loadAsset(assetManager, "ice_wizard_attack.png");
    if (!texMageIceAttack_) texMageIceAttack_ = texMageIce_;
    
    texMageLightningAttack_ = TextureAsset::loadAsset(assetManager, "lightning_wizard_atack.png");
    if (!texMageLightningAttack_) texMageLightningAttack_ = texMageLightning_;
    
    // Mage tower UI icon (for button)
    texMageIcon_ = TextureAsset::loadAsset(assetManager, "wizards_team_square.png");
    if (!texMageIcon_) texMageIcon_ = TextureAsset::createSolidColor(100, 50, 200, 255); // Purple fallback
    
    // Flying enemies
    texBat_ = TextureAsset::createSolidColor(80, 60, 100, 255); // Purple-gray
    texFlyingEye_ = TextureAsset::createSolidColor(200, 50, 100, 255); // Red-pink

    // Create models
    auto makeQuad = [this](std::shared_ptr<TextureAsset> tex) -> Model* {
        auto model = std::make_unique<Model>(quadVertices_, quadIndices_, tex);
        Model* ptr = model.get();
        models_.push_back(std::move(model));
        return ptr;
    };

    quadGrass_ = makeQuad(texGrass_);
    quadPath_ = makeQuad(texPath_);
    quadBlocked_ = makeQuad(texBlocked_);
    // Tower quads by level
    quadTowerL1_ = makeQuad(texTowerL1_);
    quadTowerL1Attack_ = makeQuad(texTowerL1Attack_);
    quadTowerL2_ = makeQuad(texTowerL2_);
    quadTowerL2Attack_ = makeQuad(texTowerL2Attack_);
    quadTowerL3_ = makeQuad(texTowerL3_);
    quadTowerL3Attack_ = makeQuad(texTowerL3Attack_);
    quadSlime_ = makeQuad(texSlime_);
    quadSlimeDeath_ = makeQuad(texSlimeDeath_);
    quadGoblin_ = makeQuad(texGoblin_);
    quadGoblinDeath_ = makeQuad(texGoblinDeath_);
    quadSwordsman_ = makeQuad(texSwordsman_);
    quadSheriff_ = makeQuad(texSheriff_);
    quadSheriffTower_ = makeQuad(texSheriff_); // Use sheriff texture for tower
    quadProjectile_ = makeQuad(texProjectile_);
    quadMenuChest_ = makeQuad(texMenuChest_);
    quadWhite_ = makeQuad(texWhite_);
    
    // Mage towers - idle
    quadMageFire_ = makeQuad(texMageFire_);
    quadMageIce_ = makeQuad(texMageIce_);
    quadMageLightning_ = makeQuad(texMageLightning_);
    
    // Mage towers - attack
    quadMageFireAttack_ = makeQuad(texMageFireAttack_);
    quadMageIceAttack_ = makeQuad(texMageIceAttack_);
    quadMageLightningAttack_ = makeQuad(texMageLightningAttack_);
    quadMageIcon_ = makeQuad(texMageIcon_);
    
    // Flying enemies
    quadBat_ = makeQuad(texBat_);
    quadFlyingEye_ = makeQuad(texFlyingEye_);

    // Setup spritesheets for pixel art animations
    // Tower level 1: 4 frames idle (192x48), 6 frames attack (288x48)
    towerL1Sheet_ = SpriteSheet(4, 1, 4);
    towerL1AttackSheet_ = SpriteSheet(6, 1, 6);
    // Tower level 2: same frame counts
    towerL2Sheet_ = SpriteSheet(4, 1, 4);
    towerL2AttackSheet_ = SpriteSheet(6, 1, 6);
    // Tower level 3: same frame counts  
    towerL3Sheet_ = SpriteSheet(4, 1, 4);
    towerL3AttackSheet_ = SpriteSheet(6, 1, 6);
    // Slime: 6 frames walk animation (288x48 = 6x48 each)
    slimeSheet_ = SpriteSheet(6, 1, 6);
    // Slime death: 6 frames (288x48 = 6x48 each)
    slimeDeathSheet_ = SpriteSheet(6, 1, 6);
    // Goblin: 6 frames walk animation (288x48 = 6x48 each)
    goblinSheet_ = SpriteSheet(6, 1, 6);
    // Goblin death: 6 frames (288x48 = 6x48 each)
    goblinDeathSheet_ = SpriteSheet(6, 1, 6);
    
    // Mage animations: 4 frames each (assuming 256x64 = 4x64 each frame)
    mageFireSheet_ = SpriteSheet(4, 1, 4);
    mageIceSheet_ = SpriteSheet(4, 1, 4);
    mageLightningSheet_ = SpriteSheet(4, 1, 4);

    // Create game instance
    game_ = std::make_unique<Game>();

    initialized_ = true;
    aout << "Game assets initialized successfully" << std::endl;
}

void Renderer::initRenderer() {
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                         << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    aout << "Found " << numConfigs << " configs" << std::endl;
    aout << "Chose " << config << std::endl;

    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    assert(madeCurrent);

    display_ = display;
    surface_ = surface;
    context_ = context;

    width_ = -1;
    height_ = -1;

    PRINT_GL_STRING(GL_VENDOR);
    PRINT_GL_STRING(GL_RENDERER);
    PRINT_GL_STRING(GL_VERSION);
    PRINT_GL_STRING_AS_LIST(GL_EXTENSIONS);

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection"));
    assert(shader_);

    shader_->activate();

    float identity[16];
    Utility::buildIdentityMatrix(identity);
    shader_->setModelMatrix(identity);
    shader_->setColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClearColor(0.15f, 0.25f, 0.1f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize projection defaults
    projHalfHeight_ = MAP_HEIGHT / 2.0f;
    projHalfWidth_ = MAP_WIDTH / 2.0f;
    projCenterX_ = MAP_WIDTH / 2.0f;
    projCenterY_ = MAP_HEIGHT / 2.0f;

    // Initialize game assets
    initGameAssets();
}

void Renderer::updateRenderArea() {
    EGLint width;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);

    EGLint height;
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);

    if (width != width_ || height != height_) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
        shaderNeedsNewProjectionMatrix_ = true;
    }
}

void Renderer::handleInput() {
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) {
        return;
    }

    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP: {
                // Convert screen coordinates to world coordinates
                float worldX, worldY;
                screenToWorld(x, y, worldX, worldY);

                aout << "Tap at screen (" << x << ", " << y
                     << ") -> world (" << worldX << ", " << worldY << ")" << std::endl;

                if (!game_) break;

                // Check tower selection buttons first
                if (hud_.handleTowerSelectionTap(worldX, worldY, MAP_WIDTH, MAP_HEIGHT, *game_)) {
                    float margin = 0.15f;
                    float costY = 0.1f;
                    float btnSize = 0.8f;
                    float archerX = margin + 0.2f;
                    float sheriffX = margin + 2.0f;
                    float btnY = costY + 0.1f;
                    
                    if (worldX >= archerX && worldX <= archerX + btnSize &&
                        worldY >= btnY && worldY <= btnY + btnSize) {
                        game_->selectArcherTower();
                        aout << "Archer tower selected" << std::endl;
                    } else if (worldX >= sheriffX && worldX <= sheriffX + btnSize &&
                               worldY >= btnY && worldY <= btnY + btnSize) {
                        game_->selectSheriffTower();
                        aout << "Sheriff tower selected" << std::endl;
                    }
                    break;
                }

                // Handle login screen
                if (game_->getState() == GameState::LOGIN) {
                    int loginTap = hud_.handleLoginTap(worldX, worldY, MAP_WIDTH, MAP_HEIGHT);
                    if (loginTap == 2) {
                        // Login button - for now simulate successful login
                        // In real implementation, this would call Firebase Auth
                        game_->login("player@example.com", "demo_uid_123");
                        aout << "Login successful (demo)" << std::endl;
                    } else if (loginTap == 3) {
                        // Register button
                        aout << "Register button tapped (demo)" << std::endl;
                        game_->login("player@example.com", "demo_uid_123");
                    } else if (loginTap == 4) {
                        // Skip/Continue as Guest
                        game_->skipLogin();
                        aout << "Continuing as guest" << std::endl;
                    }
                    break;
                }
                
                // Handle main menu
                if (game_->getState() == GameState::MENU) {
                    int menuTap = hud_.handleMainMenuTap(worldX, worldY, MAP_WIDTH, MAP_HEIGHT);
                    if (menuTap == 0) {
                        game_->startGame(); // Switch to difficulty select
                    } else if (menuTap == 1) {
                        // Buy chest
                        if (game_->buyChest()) {
                            game_->openChest();
                        }
                    } else if (menuTap == 2) {
                        // Open character menu
                        game_->openCharacterMenu();
                    } else if (menuTap == 3) {
                        // Enter sandbox mode
                        game_->enterSandbox();
                    }
                    break;
                }
                
                // Handle sandbox mode
                if (game_->getState() == GameState::SANDBOX) {
                    // Check sandbox controls first
                    int sandboxTap = hud_.handleSandboxTap(worldX, worldY, MAP_WIDTH, MAP_HEIGHT);
                    if (sandboxTap == 0) {
                        // BACK to menu
                        game_->exitSandbox();
                    } else if (sandboxTap == 1) {
                        // CLEAR all towers
                        game_->sandboxClearTowers();
                    } else if (sandboxTap == 2) {
                        // NEXT WAVE
                        game_->sandboxNextWave();
                    } else if (sandboxTap == 3) {
                        // Spawn SLIME
                        game_->sandboxSpawnEnemy(EnemyType::SLIME);
                    } else if (sandboxTap == 4) {
                        // Spawn GOBLIN
                        game_->sandboxSpawnEnemy(EnemyType::GOBLIN);
                    } else if (sandboxTap == 5) {
                        // Spawn BAT
                        game_->sandboxSpawnEnemy(EnemyType::BAT);
                    } else if (sandboxTap == 6) {
                        // Spawn BOSS
                        game_->sandboxSpawnEnemy(EnemyType::BOSS);
                    } else {
                        // Place tower on map
                        game_->onTap(worldX, worldY);
                    }
                    break;
                }
                
                // Handle character menu
                if (game_->getState() == GameState::CHARACTER_MENU) {
                    int charTap = hud_.handleCharacterMenuTap(worldX, worldY, MAP_WIDTH, MAP_HEIGHT);
                    if (charTap == 0) {
                        game_->closeCharacterMenu();
                    } else if (charTap == 1) {
                        game_->upgradeArcher();
                    } else if (charTap == 2) {
                        game_->upgradeSheriff();
                    } else if (charTap == 3) {
                        game_->upgradeAlly();
                    }
                    break;
                }
                
                if (game_->getState() == GameState::DIFFICULTY_SELECT) {
                    // Check difficulty buttons
                    int difficulty = hud_.handleDifficultyTap(worldX, worldY, MAP_WIDTH, MAP_HEIGHT);
                    if (difficulty == 0) {
                        game_->startGameWithDifficulty(Difficulty::EASY);
                    } else if (difficulty == 1) {
                        game_->startGameWithDifficulty(Difficulty::MEDIUM);
                    } else if (difficulty == 2) {
                        game_->startGameWithDifficulty(Difficulty::HARD);
                    } else if (difficulty == 3) {
                        game_->startGameWithDifficulty(Difficulty::ENDLESS);
                    }
                    break;
                }

                // Check if upgrade menu is open and tapped on upgrade button
                if (game_->isUpgradeMenuOpen()) {
                    const Tower* tower = game_->getSelectedTower();
                    if (tower && tower->getLevel() < Tower::getMaxLevel()) {
                        // Check if tap is within upgrade button area
                        float menuCenterX = MAP_WIDTH * 0.5f;
                        float menuCenterY = MAP_HEIGHT * 0.5f;
                        if (worldX >= menuCenterX - 1.5f && worldX <= menuCenterX + 1.5f &&
                            worldY >= menuCenterY - 1.2f && worldY <= menuCenterY - 0.4f) {
                            // Tapped on upgrade button
                            if (game_->upgradeSelectedTower()) {
                                aout << "Tower upgraded successfully!" << std::endl;
                            } else {
                                aout << "Cannot upgrade tower (insufficient gold or max level)" << std::endl;
                                game_->closeUpgradeMenu();
                            }
                            break;
                        }
                    }
                    // If not on button, close menu (handled in onTap)
                }

                // Check for pause button tap (top-right corner)
                // Match rendering: pauseX = mapWidth - 0.9f, pauseY = mapHeight - 0.9f, size = 0.6f
                if (worldX >= MAP_WIDTH - 0.9f && worldX <= MAP_WIDTH - 0.3f &&
                    worldY >= MAP_HEIGHT - 0.9f && worldY <= MAP_HEIGHT - 0.3f) {
                    game_->togglePause();
                    break;
                }

                // Check for pause menu buttons
                if (game_->isPaused()) {
                    float centerX = MAP_WIDTH * 0.5f;
                    float centerY = MAP_HEIGHT * 0.5f;
                    float btnWidth = 3.5f;
                    float btnHeight = 0.9f;
                    float btnX = centerX - btnWidth * 0.5f;
                    
                    // BACK button (top, y = centerY + 1.5f)
                    if (worldX >= btnX && worldX <= btnX + btnWidth &&
                        worldY >= centerY + 1.5f - btnHeight*0.5f && worldY <= centerY + 1.5f + btnHeight*0.5f) {
                        game_->resumeGame();
                        break;
                    }
                    
                    // RESTART button (middle, y = centerY)
                    if (worldX >= btnX && worldX <= btnX + btnWidth &&
                        worldY >= centerY - btnHeight*0.5f && worldY <= centerY + btnHeight*0.5f) {
                        game_->restartGame();
                        break;
                    }
                    
                    // MENU button (bottom, y = centerY - 1.5f)
                    if (worldX >= btnX && worldX <= btnX + btnWidth &&
                        worldY >= centerY - 1.5f - btnHeight*0.5f && worldY <= centerY - 1.5f + btnHeight*0.5f) {
                        game_->returnToMenu();
                        break;
                    }
                }

                game_->onTap(worldX, worldY);
                break;
            }
            default:
                break;
        }
    }
    android_app_clear_motion_events(inputBuffer);

    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        aout << "Key: " << keyEvent.keyCode << " "
             << (keyEvent.action == AKEY_EVENT_ACTION_DOWN ? "Down" : "Up") << std::endl;
    }
    android_app_clear_key_events(inputBuffer);
}
