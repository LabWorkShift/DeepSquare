#include "../include/nnue.h"
#include <fstream>
#include <cmath>

NNUE::NNUE() {
    accumulator[0].computed = false;
    accumulator[1].computed = false;
}

void NNUE::loadWeights(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    
    file.read(reinterpret_cast<char*>(weights1.data()), 
              INPUT_SIZE * HIDDEN_SIZE * sizeof(int16_t));
    file.read(reinterpret_cast<char*>(biases1.data()), 
              HIDDEN_SIZE * sizeof(int16_t));
    file.read(reinterpret_cast<char*>(weights2.data()), 
              HIDDEN_SIZE * sizeof(int16_t));
    file.read(reinterpret_cast<char*>(&bias2), sizeof(int16_t));
}

void NNUE::refreshAccumulator(const Board& board) {
    accumulator[0].computed = false;
    accumulator[1].computed = false;
    initializeAccumulator(board, true);
    initializeAccumulator(board, false);
}

void NNUE::updateAccumulator(const Board& board, const Move& move) {
    Piece movedPiece = board.getPiece(move.fromX, move.fromY);
    Piece capturedPiece = board.getPiece(move.toX, move.toY);
    
    int fromSquare = move.fromY * 8 + move.fromX;
    int toSquare = move.toY * 8 + move.toX;
    
    for(bool perspective : {true, false}) {
        if(!accumulator[perspective].computed) continue;
        
        int fromIdx = getFeatureIndex(movedPiece, fromSquare, perspective);
        int toIdx = getFeatureIndex(movedPiece, toSquare, perspective);
        
        for(int i = 0; i < HIDDEN_SIZE; ++i) {
            accumulator[perspective].values[i] -= weights1[fromIdx][i];
            accumulator[perspective].values[i] += weights1[toIdx][i];
            
            if(capturedPiece.getType() != EMPTY) {
                int capturedIdx = getFeatureIndex(capturedPiece, toSquare, perspective);
                accumulator[perspective].values[i] -= weights1[capturedIdx][i];
            }
        }
    }
}

int NNUE::evaluate(const Board& board, bool perspective) {
    if(!accumulator[perspective].computed) {
        initializeAccumulator(board, perspective);
    }
    
    int32_t sum = 0;
    for(int i = 0; i < HIDDEN_SIZE; ++i) {
        sum += accumulator[perspective].values[i] * weights2[i];
    }
    sum = sum / 64 + bias2;
    
    return perspective ? sum : -sum;
}

int16_t NNUE::clamp(int32_t x) {
    return static_cast<int16_t>(std::max<int32_t>(-32768, std::min<int32_t>(32767, x)));
}

int NNUE::getFeatureIndex(const Piece& piece, int square, bool perspective) const {
    int pieceIndex = piece.getType() * 2 + (piece.getColor() == WHITE ? 0 : 1);
    if(!perspective) square = 63 - square;
    return pieceIndex * 64 + square;
}

void NNUE::initializeAccumulator(const Board& board, bool perspective) {
    std::fill(accumulator[perspective].values.begin(), 
              accumulator[perspective].values.end(), 0);
              
    for(int y = 0; y < 8; ++y) {
        for(int x = 0; x < 8; ++x) {
            Piece piece = board.getPiece(x, y);
            if(piece.getType() != EMPTY) {
                int square = y * 8 + x;
                int featureIndex = getFeatureIndex(piece, square, perspective);
                
                for(int i = 0; i < HIDDEN_SIZE; ++i) {
                    accumulator[perspective].values[i] += weights1[featureIndex][i];
                }
            }
        }
    }
    
    for(int i = 0; i < HIDDEN_SIZE; ++i) {
        accumulator[perspective].values[i] = 
            clamp(accumulator[perspective].values[i] + biases1[i]);
    }
    
    accumulator[perspective].computed = true;
} 