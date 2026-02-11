#include "ParticleSystem.h"
#include "Shader.h"
#include "Model.h"
#include <cmath>
#include <cstring>

Particle::Particle(float x, float y, ParticleType t) 
    : x(x), y(y), type(t), lifetime(0.0f) {
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> velDist(-2.0f, 2.0f);
    std::uniform_real_distribution<float> lifeDist(0.3f, 0.8f);
    std::uniform_real_distribution<float> sizeDist(0.05f, 0.15f);
    
    vx = velDist(rng);
    vy = velDist(rng);
    maxLifetime = lifeDist(rng);
    size = sizeDist(rng);
    
    // Set colors based on type
    switch (type) {
        case ParticleType::EXPLOSION:
            r = 1.0f; g = 0.5f + (rand() % 50) / 100.0f; b = 0.0f; a = 1.0f;
            break;
        case ParticleType::SPARK:
            r = 1.0f; g = 1.0f; b = 0.5f; a = 1.0f;
            size *= 0.5f;
            break;
        case ParticleType::BLOOD:
            r = 0.2f + (rand() % 30) / 100.0f; 
            g = 0.6f + (rand() % 40) / 100.0f; 
            b = 0.1f; a = 0.8f;
            break;
        case ParticleType::ICE:
            r = 0.7f; g = 0.9f; b = 1.0f; a = 0.9f;
            break;
        case ParticleType::FIRE:
            r = 1.0f; g = 0.4f + (rand() % 40) / 100.0f; b = 0.0f; a = 0.9f;
            vy = 1.0f + (rand() % 100) / 100.0f; // Float up
            vx *= 0.3f;
            break;
        case ParticleType::SMOKE:
            r = 0.4f; g = 0.4f; b = 0.4f; a = 0.6f;
            vy = 0.5f + (rand() % 50) / 100.0f; // Float up slowly
            size *= 2.0f;
            break;
    }
}

void Particle::update(float dt) {
    lifetime += dt;
    x += vx * dt;
    y += vy * dt;
    
    // Gravity for some types
    if (type == ParticleType::BLOOD || type == ParticleType::EXPLOSION) {
        vy -= 3.0f * dt;
    }
    
    // Fade out
    if (lifetime > maxLifetime * 0.7f) {
        a = 1.0f - (lifetime - maxLifetime * 0.7f) / (maxLifetime * 0.3f);
    }
    
    // Shrink
    size *= 0.98f;
}

void ParticleSystem::spawnExplosion(float x, float y, int count) {
    spawnParticles(x, y, ParticleType::EXPLOSION, count);
}

void ParticleSystem::spawnSpark(float x, float y, int count) {
    spawnParticles(x, y, ParticleType::SPARK, count);
}

void ParticleSystem::spawnBlood(float x, float y, int count) {
    spawnParticles(x, y, ParticleType::BLOOD, count);
}

void ParticleSystem::spawnIce(float x, float y, int count) {
    spawnParticles(x, y, ParticleType::ICE, count);
}

void ParticleSystem::spawnFire(float x, float y, int count) {
    spawnParticles(x, y, ParticleType::FIRE, count);
}

void ParticleSystem::spawnSmoke(float x, float y, int count) {
    spawnParticles(x, y, ParticleType::SMOKE, count);
}

void ParticleSystem::spawnParticles(float x, float y, ParticleType type, int count) {
    for (int i = 0; i < count; i++) {
        spawnSingle(x, y, type);
    }
}

void ParticleSystem::spawnSingle(float x, float y, ParticleType type) {
    particles_.emplace_back(x, y, type);
}

void ParticleSystem::update(float dt) {
    for (auto& p : particles_) {
        p.update(dt);
    }
    
    // Remove dead particles
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) { return !p.isAlive(); }),
        particles_.end()
    );
}

void ParticleSystem::render(Shader& shader, const Model& quad) {
    float modelMatrix[16];
    
    for (const auto& p : particles_) {
        if (p.a <= 0.0f) continue;
        
        memset(modelMatrix, 0, sizeof(modelMatrix));
        modelMatrix[0] = p.size;
        modelMatrix[5] = p.size;
        modelMatrix[10] = 1.0f;
        modelMatrix[12] = p.x;
        modelMatrix[13] = p.y;
        
        shader.setModelMatrix(modelMatrix);
        shader.setColor(p.r, p.g, p.b, p.a);
        shader.drawModel(quad);
    }
}
