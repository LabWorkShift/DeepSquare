#include "../include/move.h"

Move::Move(int fx, int fy, int tx, int ty, int s) {
    fromX = fx;
    fromY = fy;
    toX = tx;
    toY = ty;
    promotion = EMPTY;
    score = s;
}

bool Move::operator<(const Move& other) const {
    return score < other.score;
} 