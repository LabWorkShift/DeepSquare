#include "../include/board.h"

Board::Board() : whiteToMove(true) {
    initialize();
}

void Board::initialize() {
    
    board[0][0] = Piece(ROOK, WHITE);
    board[0][1] = Piece(KNIGHT, WHITE);
    board[0][2] = Piece(BISHOP, WHITE);
    board[0][3] = Piece(QUEEN, WHITE);
    board[0][4] = Piece(KING, WHITE);
    board[0][5] = Piece(BISHOP, WHITE);
    board[0][6] = Piece(KNIGHT, WHITE);
    board[0][7] = Piece(ROOK, WHITE);
    
    for(int x = 0; x < 8; x++) {
        board[1][x] = Piece(PAWN, WHITE);
    }
    
    
    board[7][0] = Piece(ROOK, BLACK);
    board[7][1] = Piece(KNIGHT, BLACK);
    board[7][2] = Piece(BISHOP, BLACK);
    board[7][3] = Piece(QUEEN, BLACK);
    board[7][4] = Piece(KING, BLACK);
    board[7][5] = Piece(BISHOP, BLACK);
    board[7][6] = Piece(KNIGHT, BLACK);
    board[7][7] = Piece(ROOK, BLACK);
    
    for(int x = 0; x < 8; x++) {
        board[6][x] = Piece(PAWN, BLACK);
    }
    
   
    for(int y = 2; y < 6; y++) {
        for(int x = 0; x < 8; x++) {
            board[y][x] = Piece();
        }
    }
}

Piece Board::getPiece(int x, int y) const {
    if(x < 0 || x >= 8 || y < 0 || y >= 8) return Piece();
    return board[y][x];
}

bool Board::makeMove(int fromX, int fromY, int toX, int toY, PieceType promotion) {
    if(fromX < 0 || fromX >= 8 || fromY < 0 || fromY >= 8 ||
       toX < 0 || toX >= 8 || toY < 0 || toY >= 8) {
        return false;
    }
    
    Piece& piece = board[fromY][fromX];
    bool isCapture = board[toY][toX].getType() != EMPTY;
    
    if(piece.getType() == EMPTY || 
       (piece.getColor() == WHITE) != whiteToMove ||
       !piece.isValidMove(fromX, fromY, toX, toY, isCapture)) {
        return false;
    }
    
    // Handle pawn promotion
    if(piece.getType() == PAWN && (toY == 0 || toY == 7)) {
        if(promotion == EMPTY) {
            promotion = QUEEN; // Default promotion to queen
        }
        board[toY][toX] = Piece(promotion, piece.getColor());
        board[fromY][fromX] = Piece();
    } else {
        // Make normal move
        board[toY][toX] = piece;
        board[fromY][fromX] = Piece();
    }
    
    whiteToMove = !whiteToMove;
    
    // Check if the move puts own king in check
    if(isCheck()) {
        // Undo the move
        board[fromY][fromX] = piece;
        board[toY][toX] = Piece();
        whiteToMove = !whiteToMove;
        return false;
    }
    
    return true;
}

bool Board::isCheck() const {
    // Find king position
    int kingX = -1, kingY = -1;
    Color currentColor = whiteToMove ? WHITE : BLACK;
    
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            Piece piece = board[y][x];
            if(piece.getType() == KING && piece.getColor() == currentColor) {
                kingX = x;
                kingY = y;
                break;
            }
        }
        if(kingX != -1) break;
    }
    
    // Check if any opponent piece can capture the king
    for(int y = 0; y < 8; y++) {
        for(int x = 0; x < 8; x++) {
            Piece piece = board[y][x];
            if(piece.getType() != EMPTY && piece.getColor() != currentColor) {
                if(piece.isValidMove(x, y, kingX, kingY, true)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool Board::isCheckmate() const {
    if(!isCheck()) return false;
    
    // Try all possible moves
    for(int fromY = 0; fromY < 8; fromY++) {
        for(int fromX = 0; fromX < 8; fromX++) {
            Piece piece = board[fromY][fromX];
            if(piece.getType() != EMPTY && 
               (piece.getColor() == WHITE) == whiteToMove) {
                for(int toY = 0; toY < 8; toY++) {
                    for(int toX = 0; toX < 8; toX++) {
                        Board tempBoard = *this;
                        if(tempBoard.makeMove(fromX, fromY, toX, toY)) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

void Board::setFromFEN(const std::string& fen) {
    int x = 0, y = 7;
    
    for(char c : fen) {
        if(c == '/') {
            y--;
            x = 0;
        }
        else if(c >= '1' && c <= '8') {
            x += c - '0';
        }
        else {
            PieceType type;
            Color color = (c >= 'A' && c <= 'Z') ? WHITE : BLACK;
            char piece = (color == WHITE) ? c : c - ('a' - 'A');
            
            switch(piece) {
                case 'P': type = PAWN; break;
                case 'N': type = KNIGHT; break;
                case 'B': type = BISHOP; break;
                case 'R': type = ROOK; break;
                case 'Q': type = QUEEN; break;
                case 'K': type = KING; break;
                default: continue;
            }
            
            board[y][x] = Piece(type, color);
            x++;
        }
    }
}

std::vector<std::pair<int, int>> Board::getLegalMoves(int x, int y) const {
    std::vector<std::pair<int, int>> moves;
    Piece piece = getPiece(x, y);
    
    if(piece.getType() == EMPTY || (piece.getColor() == WHITE) != whiteToMove) {
        return moves;
    }
    
    for(int toY = 0; toY < 8; toY++) {
        for(int toX = 0; toX < 8; toX++) {
            bool isCapture = getPiece(toX, toY).getType() != EMPTY;
            if(piece.isValidMove(x, y, toX, toY, isCapture)) {
                Board tempBoard = *this;
                if(tempBoard.makeMove(x, y, toX, toY)) {
                    moves.emplace_back(toX, toY);
                }
            }
        }
    }
    
    return moves;
} 