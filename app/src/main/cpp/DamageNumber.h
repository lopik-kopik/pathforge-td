#ifndef TOWERDEFENSE_DAMAGENUMBER_H
#define TOWERDEFENSE_DAMAGENUMBER_H

#include <vector>
#include <string>

class Shader;
class Model;

struct DamageNumber {
    float x, y;           // Position
    int damage;           // Damage value
    float lifetime;       // Current lifetime
    float maxLifetime;    // Max lifetime (seconds)
    float velocityY;      // Upward velocity
    bool isCritical;      // Critical hit (bigger, red)
    bool isBurn;          // Burn damage (orange)
    
    DamageNumber(float x, float y, int dmg, bool crit = false, bool burn = false)
        : x(x), y(y), damage(dmg), lifetime(0.0f), maxLifetime(1.0f),
          velocityY(1.5f), isCritical(crit), isBurn(burn) {}
    
    bool isAlive() const { return lifetime < maxLifetime; }
    void update(float dt);
    float getAlpha() const;
    float getScale() const;
};

class DamageNumberManager {
public:
    void spawn(float x, float y, int damage, bool critical = false, bool burn = false);
    void update(float dt);
    void render(Shader& shader, const Model& quad);
    void clear() { numbers_.clear(); }
    
private:
    std::vector<DamageNumber> numbers_;
    
    void drawNumber(Shader& shader, const Model& quad, const DamageNumber& num);
    void drawDigit(Shader& shader, const Model& quad, float x, float y, float size, int digit, float r, float g, float b, float a);
};

#endif //TOWERDEFENSE_DAMAGENUMBER_H
