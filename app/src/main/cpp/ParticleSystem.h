#ifndef TOWERDEFENSE_PARTICLESYSTEM_H
#define TOWERDEFENSE_PARTICLESYSTEM_H

#include <vector>
#include <random>

class Shader;
class Model;

enum class ParticleType {
    EXPLOSION,  // Red/orange burst
    SPARK,      // Yellow/white electric
    BLOOD,      // Green/purple enemy blood
    ICE,        // Blue/white ice shards
    FIRE,       // Orange/red fire embers
    SMOKE       // Gray smoke
};

struct Particle {
    float x, y;
    float vx, vy;
    float lifetime;
    float maxLifetime;
    float size;
    float r, g, b, a;
    ParticleType type;
    
    Particle(float x, float y, ParticleType type);
    void update(float dt);
    bool isAlive() const { return lifetime < maxLifetime; }
};

class ParticleSystem {
public:
    void spawnExplosion(float x, float y, int count = 10);
    void spawnSpark(float x, float y, int count = 5);
    void spawnBlood(float x, float y, int count = 8);
    void spawnIce(float x, float y, int count = 6);
    void spawnFire(float x, float y, int count = 4);
    void spawnSmoke(float x, float y, int count = 3);
    
    void spawnParticles(float x, float y, ParticleType type, int count);
    void update(float dt);
    void render(Shader& shader, const Model& quad);
    void clear() { particles_.clear(); }
    
private:
    std::vector<Particle> particles_;
    std::mt19937 rng_{std::random_device{}()};
    
    void spawnSingle(float x, float y, ParticleType type);
};

#endif //TOWERDEFENSE_PARTICLESYSTEM_H
