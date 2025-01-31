#include "../include/evaluation.h"
#include "../include/nnue.h"

int Evaluation::evaluatePosition(const Board& board) {
    static NNUE nnue;
    static bool initialized = false;
    
    if(!initialized) {
        nnue.loadWeights("weights.bin");
        initialized = true;
    }
    
    return nnue.evaluate(board, board.isWhiteToMove());
}

int Evaluation::getPieceValue(const Piece& piece) {
    return piece.getValue();
}

int Evaluation::getPositionalValue(const Piece& piece, int x, int y) {
    static const int pawnTable[8][8] = {
        {0,  0,  0,  0,  0,  0,  0,  0},
        {50, 50, 50, 50, 50, 50, 50, 50},
        {10, 10, 20, 30, 30, 20, 10, 10},
        {5,  5, 10, 25, 25, 10,  5,  5},
        {0,  0,  0, 20, 20,  0,  0,  0},
        {5, -5,-10,  0,  0,-10, -5,  5},
        {5, 10, 10,-20,-20, 10, 10,  5},
        {0,  0,  0,  0,  0,  0,  0,  0}
    };
    
    if(piece.getType() == PAWN) {
        return pawnTable[piece.getColor() == WHITE ? y : 7-y][x];
    }
    
    return 0;
} 