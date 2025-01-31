#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
#include <vector>
#include <string>

class Board {
private:
    Piece board[8][8];
    bool whiteToMove;
    
public:
    Board();
    void initialize();
    Piece getPiece(int x, int y) const;
    bool makeMove(int fromX, int fromY, int toX, int toY, PieceType promotion = EMPTY);
    bool isCheck() const;
    bool isCheckmate() const;
    bool isWhiteToMove() const { return whiteToMove; }
    void setFromFEN(const std::string& fen);
    std::vector<std::pair<int, int>> getLegalMoves(int x, int y) const;
};

#endif 