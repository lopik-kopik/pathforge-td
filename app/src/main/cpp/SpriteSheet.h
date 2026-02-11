#ifndef TOWERDEFENSE_SPRITESHEET_H
#define TOWERDEFENSE_SPRITESHEET_H

struct SpriteFrame {
    float u0, v0; // top-left UV
    float u1, v1; // bottom-right UV
};

class SpriteSheet {
public:
    SpriteSheet() : cols_(1), rows_(1), totalFrames_(1) {}
    SpriteSheet(int cols, int rows, int totalFrames = -1);

    SpriteFrame getFrame(int index) const;
    int getTotalFrames() const { return totalFrames_; }

private:
    int cols_;
    int rows_;
    int totalFrames_;
};

#endif //TOWERDEFENSE_SPRITESHEET_H
