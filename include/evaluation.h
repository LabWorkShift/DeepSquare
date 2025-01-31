#ifndef EVALUATION_H
#define EVALUATION_H

#include "board.h"

class Evaluation {
public:
    static int evaluatePosition(const Board& board);
    static int getPieceValue(const Piece& piece);
    static int getPositionalValue(const Piece& piece, int x, int y);
};

#endif 