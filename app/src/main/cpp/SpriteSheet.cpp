#include "SpriteSheet.h"

SpriteSheet::SpriteSheet(int cols, int rows, int totalFrames)
    : cols_(cols), rows_(rows) {
    totalFrames_ = (totalFrames > 0) ? totalFrames : cols * rows;
}

SpriteFrame SpriteSheet::getFrame(int index) const {
    if (index < 0) index = 0;
    if (index >= totalFrames_) index = totalFrames_ - 1;

    int col = index % cols_;
    int row = index / cols_;

    float fw = 1.0f / (float)cols_;
    float fh = 1.0f / (float)rows_;

    SpriteFrame frame;
    frame.u0 = col * fw;
    frame.v0 = row * fh;
    frame.u1 = (col + 1) * fw;
    frame.v1 = (row + 1) * fh;

    return frame;
}
