#include "../include/nnue.h"
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace simd;

NNUE::NNUE() : accumulatorStackSize(0) {
    accumulator[0].computed = false;
    accumulator[1].computed = false;
    simdBuffer = std::make_unique<uint8_t[]>(32 * HIDDEN_SIZE * sizeof(int16_t));
}

NNUE::~NNUE() = default;

NNUE::NNUE(const NNUE& other) : 
    featureWeights(other.featureWeights),
    outputWeights(other.outputWeights),
    outputBias(other.outputBias),
    accumulator(other.accumulator),
    accumulatorStack(other.accumulatorStack),
    accumulatorStackSize(other.accumulatorStackSize) {
    simdBuffer = std::make_unique<uint8_t[]>(32 * HIDDEN_SIZE * sizeof(int16_t));
    std::copy(
        other.simdBuffer.get(),
        other.simdBuffer.get() + (32 * HIDDEN_SIZE * sizeof(int16_t)),
        simdBuffer.get()
    );
}

NNUE& NNUE::operator=(const NNUE& other) {
    if (this != &other) {
        featureWeights = other.featureWeights;
        outputWeights = other.outputWeights;
        outputBias = other.outputBias;
        accumulator = other.accumulator;
        accumulatorStack = other.accumulatorStack;
        accumulatorStackSize = other.accumulatorStackSize;
        
        simdBuffer = std::make_unique<uint8_t[]>(32 * HIDDEN_SIZE * sizeof(int16_t));
        std::copy(
            other.simdBuffer.get(),
            other.simdBuffer.get() + (32 * HIDDEN_SIZE * sizeof(int16_t)),
            simdBuffer.get()
        );
    }
    return *this;
}

NNUE::NNUE(NNUE&& other) noexcept :
    featureWeights(std::move(other.featureWeights)),
    outputWeights(std::move(other.outputWeights)),
    outputBias(other.outputBias),
    accumulator(std::move(other.accumulator)),
    accumulatorStack(std::move(other.accumulatorStack)),
    accumulatorStackSize(other.accumulatorStackSize),
    simdBuffer(std::move(other.simdBuffer)) {
}

NNUE& NNUE::operator=(NNUE&& other) noexcept {
    if (this != &other) {
        featureWeights = std::move(other.featureWeights);
        outputWeights = std::move(other.outputWeights);
        outputBias = other.outputBias;
        accumulator = std::move(other.accumulator);
        accumulatorStack = std::move(other.accumulatorStack);
        accumulatorStackSize = other.accumulatorStackSize;
        simdBuffer = std::move(other.simdBuffer);
    }
    return *this;
}

void NNUE::loadWeights(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return;

    uint32_t numFeatures;
    file.read(reinterpret_cast<char*>(&numFeatures), sizeof(numFeatures));
    
    featureWeights.resize(numFeatures);
    for (auto& weights : featureWeights) {
        file.read(reinterpret_cast<char*>(weights.weights.data()), 
                 HIDDEN_SIZE * sizeof(int16_t));
        file.read(reinterpret_cast<char*>(&weights.bias), sizeof(int16_t));
    }
    
    file.read(reinterpret_cast<char*>(outputWeights.data()), 
              HIDDEN_SIZE * sizeof(int16_t));
    file.read(reinterpret_cast<char*>(&outputBias), sizeof(int16_t));
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
        
        int kingSquare = accumulator[perspective].kingSquare;
        int fromIdx = getFeatureIndex(movedPiece, fromSquare, kingSquare, perspective);
        int toIdx = getFeatureIndex(movedPiece, toSquare, kingSquare, perspective);
        
        for(int i = 0; i < HIDDEN_SIZE / 16; ++i) {
            vec_type acc = VectorOps::load(accumulator[perspective].values.data() + i * 16);
            vec_type fromW = VectorOps::load(featureWeights[fromIdx].weights.data() + i * 16);
            vec_type toW = VectorOps::load(featureWeights[toIdx].weights.data() + i * 16);
            
            acc = VectorOps::sub(acc, fromW);
            acc = VectorOps::add(acc, toW);
            
            if(capturedPiece.getType() != EMPTY) {
                int capturedIdx = getFeatureIndex(capturedPiece, toSquare, kingSquare, perspective);
                vec_type capturedW = VectorOps::load(featureWeights[capturedIdx].weights.data() + i * 16);
                acc = VectorOps::sub(acc, capturedW);
            }
            
            VectorOps::store(accumulator[perspective].values.data() + i * 16, acc);
        }
    }
}

void NNUE::pushAccumulator() {
    if(accumulatorStackSize < 32) {
        accumulatorStack[accumulatorStackSize] = accumulator[0];
        accumulatorStack[accumulatorStackSize + 1] = accumulator[1];
        accumulatorStackSize += 2;
    }
}

void NNUE::popAccumulator() {
    if(accumulatorStackSize >= 2) {
        accumulatorStackSize -= 2;
        accumulator[0] = accumulatorStack[accumulatorStackSize];
        accumulator[1] = accumulatorStack[accumulatorStackSize + 1];
    }
}

int NNUE::evaluate(const Board& board, bool perspective) {
    if(!accumulator[perspective].computed) {
        initializeAccumulator(board, perspective);
    }
    
    int32_t finalSum = 0;
    for(int i = 0; i < HIDDEN_SIZE / 16; ++i) {
        vec_type acc = VectorOps::load(accumulator[perspective].values.data() + i * 16);
        vec_type weight = VectorOps::load(outputWeights.data() + i * 16);
        vec_type prod = VectorOps::mul(acc, weight);
        finalSum += VectorOps::horizontal_add(prod);
    }
    
    finalSum = finalSum / 64 + outputBias;
    return perspective ? finalSum : -finalSum;
}

int16_t NNUE::clamp(int32_t x) {
    return static_cast<int16_t>(std::max<int32_t>(-32768, std::min<int32_t>(32767, x)));
}

int NNUE::getFeatureIndex(const Piece& piece, int square, int kingSquare, bool perspective) const {
    if(!perspective) {
        square = 63 - square;
        kingSquare = 63 - kingSquare;
    }
    
    int pieceIndex = piece.getType() * 2 + (piece.getColor() == WHITE ? 0 : 1);
    return kingSquare * INPUTS_PER_KING + pieceIndex * 64 + square;
}

void NNUE::initializeAccumulator(const Board& board, bool perspective) {
    std::fill(accumulator[perspective].values.begin(), 
              accumulator[perspective].values.end(), 0);
              
    int kingSquare = -1;
    for(int y = 0; y < 8 && kingSquare == -1; ++y) {
        for(int x = 0; x < 8 && kingSquare == -1; ++x) {
            Piece piece = board.getPiece(x, y);
            if(piece.getType() == KING && piece.getColor() == (perspective ? WHITE : BLACK)) {
                kingSquare = y * 8 + x;
            }
        }
    }
    
    accumulator[perspective].kingSquare = kingSquare;
    
    for(int y = 0; y < 8; ++y) {
        for(int x = 0; x < 8; ++x) {
            Piece piece = board.getPiece(x, y);
            if(piece.getType() != EMPTY) {
                int square = y * 8 + x;
                int featureIndex = getFeatureIndex(piece, square, kingSquare, perspective);
                
                for(int i = 0; i < HIDDEN_SIZE / 16; ++i) {
                    vec_type acc = VectorOps::load(accumulator[perspective].values.data() + i * 16);
                    vec_type weight = VectorOps::load(featureWeights[featureIndex].weights.data() + i * 16);
                    acc = VectorOps::add(acc, weight);
                    VectorOps::store(accumulator[perspective].values.data() + i * 16, acc);
                }
            }
        }
    }
    
    applyBatchActivation(accumulator[perspective].values.data(), HIDDEN_SIZE);
    accumulator[perspective].computed = true;
}

void NNUE::applyBatchActivation(int16_t* values, size_t size) {
    for(size_t i = 0; i < size / 16; ++i) {
        vec_type val = VectorOps::load(values + i * 16);
        vec_type zero = VectorOps::zero();
        vec_type max_val = VectorOps::set1(32767);
        val = VectorOps::min(VectorOps::max(val, zero), max_val);
        VectorOps::store(values + i * 16, val);
    }
}

simd::vec_type* NNUE::getSIMDBuffer() {
    return reinterpret_cast<simd::vec_type*>(simdBuffer.get());
} 