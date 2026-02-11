#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <memory>
#include <chrono>

#include "Model.h"
#include "Shader.h"
#include "Game.h"
#include "HUD.h"
#include "SpriteSheet.h"

struct android_app;

class Renderer {
public:
    inline Renderer(android_app *pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(0),
            height_(0),
            shaderNeedsNewProjectionMatrix_(true),
            lastFrameTime_(std::chrono::steady_clock::now()),
            initialized_(false) {
        initRenderer();
    }

    virtual ~Renderer();

    void handleInput();
    void render();

private:
    void initRenderer();
    void updateRenderArea();
    void initGameAssets();

    void renderMap();
    void renderTowers();
    void renderSheriffTowers();
    void renderMageTowers();
    void renderAllies();
    void renderFlyingEnemies();
    void renderEnemies();
    void renderProjectiles();
    void renderHUD();

    void buildModelMatrix(float *outMatrix, float x, float y, float sx, float sy);
    void screenToWorld(float screenX, float screenY, float &worldX, float &worldY);

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    bool shaderNeedsNewProjectionMatrix_;
    std::chrono::steady_clock::time_point lastFrameTime_;
    bool initialized_;

    std::unique_ptr<Shader> shader_;

    // Unit quad model (vertices from -1 to 1) - reused for everything
    std::vector<Vertex> quadVertices_;
    std::vector<Index> quadIndices_;

    // Game
    std::unique_ptr<Game> game_;
    HUD hud_;

    // Textures
    std::shared_ptr<TextureAsset> texGrass_;
    std::shared_ptr<TextureAsset> texPath_;
    std::shared_ptr<TextureAsset> texBlocked_;
    // Tower textures by level
    std::shared_ptr<TextureAsset> texTowerL1_;
    std::shared_ptr<TextureAsset> texTowerL1Attack_;
    std::shared_ptr<TextureAsset> texTowerL2_;
    std::shared_ptr<TextureAsset> texTowerL2Attack_;
    std::shared_ptr<TextureAsset> texTowerL3_;
    std::shared_ptr<TextureAsset> texTowerL3Attack_;
    std::shared_ptr<TextureAsset> texSlime_;
    std::shared_ptr<TextureAsset> texSlimeDeath_;
    std::shared_ptr<TextureAsset> texGoblin_;
    std::shared_ptr<TextureAsset> texGoblinDeath_;
    std::shared_ptr<TextureAsset> texSwordsman_;
    std::shared_ptr<TextureAsset> texSheriff_;
    std::shared_ptr<TextureAsset> texProjectile_;
    std::shared_ptr<TextureAsset> texMenuChest_; // Chest image for main menu
    std::shared_ptr<TextureAsset> texWhite_; // 1x1 white pixel for colored quads
    
    // Mage towers - idle animations
    std::shared_ptr<TextureAsset> texMageFire_;
    std::shared_ptr<TextureAsset> texMageIce_;
    std::shared_ptr<TextureAsset> texMageLightning_;
    
    // Mage towers - attack animations
    std::shared_ptr<TextureAsset> texMageFireAttack_;
    std::shared_ptr<TextureAsset> texMageIceAttack_;
    std::shared_ptr<TextureAsset> texMageLightningAttack_;
    
    // Mage tower UI icon
    std::shared_ptr<TextureAsset> texMageIcon_;
    
    // Flying enemies
    std::shared_ptr<TextureAsset> texBat_;
    std::shared_ptr<TextureAsset> texFlyingEye_;

    // Models with different textures (non-owning pointers into models_ vector)
    Model* quadGrass_ = nullptr;
    Model* quadPath_ = nullptr;
    Model* quadBlocked_ = nullptr;
    // Tower models by level
    Model* quadTowerL1_ = nullptr;
    Model* quadTowerL1Attack_ = nullptr;
    Model* quadTowerL2_ = nullptr;
    Model* quadTowerL2Attack_ = nullptr;
    Model* quadTowerL3_ = nullptr;
    Model* quadTowerL3Attack_ = nullptr;
    Model* quadSlime_ = nullptr;
    Model* quadSlimeDeath_ = nullptr;
    Model* quadGoblin_ = nullptr;
    Model* quadGoblinDeath_ = nullptr;
    Model* quadSwordsman_ = nullptr;
    Model* quadSheriff_ = nullptr;
    Model* quadSheriffTower_ = nullptr;
    Model* quadProjectile_ = nullptr;
    Model* quadMenuChest_ = nullptr;
    Model* quadWhite_ = nullptr;
    
    // Mage towers - idle
    Model* quadMageFire_ = nullptr;
    Model* quadMageIce_ = nullptr;
    Model* quadMageLightning_ = nullptr;
    
    // Mage towers - attack
    Model* quadMageFireAttack_ = nullptr;
    Model* quadMageIceAttack_ = nullptr;
    Model* quadMageLightningAttack_ = nullptr;
    
    // Mage tower UI icon
    Model* quadMageIcon_ = nullptr;
    
    // Flying enemies
    Model* quadBat_ = nullptr;
    Model* quadFlyingEye_ = nullptr;

    std::vector<std::unique_ptr<Model>> models_; // owns all models

    // Spritesheet info for animations
    SpriteSheet towerL1Sheet_;
    SpriteSheet towerL1AttackSheet_;
    SpriteSheet towerL2Sheet_;
    SpriteSheet towerL2AttackSheet_;
    SpriteSheet towerL3Sheet_;
    SpriteSheet towerL3AttackSheet_;
    SpriteSheet slimeSheet_;
    SpriteSheet slimeDeathSheet_;
    SpriteSheet goblinSheet_;
    SpriteSheet goblinDeathSheet_;
    
    // Mage spritesheets
    SpriteSheet mageFireSheet_;
    SpriteSheet mageIceSheet_;
    SpriteSheet mageLightningSheet_;
    
    // Animation timers
    float towerAnimTimer_ = 0.0f;
    float slimeAnimTimer_ = 0.0f;
    float goblinAnimTimer_ = 0.0f;

    // Projection info
    float projHalfHeight_ = 8.0f;
    float projHalfWidth_ = 5.0f;
    float projCenterX_ = 5.0f;
    float projCenterY_ = 8.0f;
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H
