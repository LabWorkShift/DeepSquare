#include "../include/piece.h"
#include <cstdlib>

Piece::Piece() : type(EMPTY), color(WHITE) {}

Piece::Piece(PieceType t, Color c) : type(t), color(c) {}

PieceType Piece::getType() const {
    return type;
}

Color Piece::getColor() const {
    return color;
}

int Piece::getValue() const {
    switch(type) {
        case PAWN: return 100;
        case KNIGHT: return 320;
        case BISHOP: return 330;
        case ROOK: return 500;
        case QUEEN: return 900;
        case KING: return 20000;
        default: return 0;
    }
}

bool Piece::isValidMove(int fromX, int fromY, int toX, int toY, bool isCapture) const {
    int dx = toX - fromX;
    int dy = toY - fromY;
    
    switch(getType()) {
        case PAWN:
            if(getColor() == WHITE) {
                if(!isCapture) {
                    return (dx == 0 && dy == 1) || (dx == 0 && dy == 2 && fromY == 1);
                } else {
                    return std::abs(dx) == 1 && dy == 1;
                }
            } else {
                if(!isCapture) {
                    return (dx == 0 && dy == -1) || (dx == 0 && dy == -2 && fromY == 6);
                } else {
                    return std::abs(dx) == 1 && dy == -1;
                }
            }
            
        case KNIGHT:
            return (std::abs(dx) == 2 && std::abs(dy) == 1) || (std::abs(dx) == 1 && std::abs(dy) == 2);
            
        case BISHOP:
            return std::abs(dx) == std::abs(dy);
            
        case ROOK:
            return dx == 0 || dy == 0;
            
        case QUEEN:
            return dx == 0 || dy == 0 || std::abs(dx) == std::abs(dy);
            
        case KING:
            return std::abs(dx) <= 1 && std::abs(dy) <= 1;
            
        default:
            return false;
    }
} 