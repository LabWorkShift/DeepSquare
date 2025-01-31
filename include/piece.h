#ifndef PIECE_H
#define PIECE_H

enum PieceType {
    EMPTY,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum Color {
    WHITE,
    BLACK
};

class Piece {
private:
    PieceType type;
    Color color;
    
public:
    Piece();
    Piece(PieceType t, Color c);
    
    PieceType getType() const;
    Color getColor() const;
    int getValue() const;
    bool isValidMove(int fromX, int fromY, int toX, int toY, bool isCapture) const;
};

#endif 