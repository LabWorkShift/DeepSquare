#ifndef MOVE_H
#define MOVE_H

#include "piece.h"

struct Move {
    int fromX;
    int fromY;
    int toX;
    int toY;
    PieceType promotion;
    int score;
    
    Move(int fx = 0, int fy = 0, int tx = 0, int ty = 0, int s = 0);
    bool operator<(const Move& other) const;
};

#endif 