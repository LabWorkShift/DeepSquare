#include "../include/nnue.h"
#include <fstream>
#include <cmath>
#include <algorithm>

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
        
        __m256i* accPtr = reinterpret_cast<__m256i*>(accumulator[perspective].values.data());
        __m256i* weightFromPtr = reinterpret_cast<__m256i*>(featureWeights[fromIdx].weights.data());
        __m256i* weightToPtr = reinterpret_cast<__m256i*>(featureWeights[toIdx].weights.data());
        
        for(int i = 0; i < HIDDEN_SIZE / 16; ++i) {
            __m256i acc = _mm256_load_si256(accPtr + i);
            __m256i fromW = _mm256_load_si256(weightFromPtr + i);
            __m256i toW = _mm256_load_si256(weightToPtr + i);
            
            acc = _mm256_sub_epi16(acc, fromW);
            acc = _mm256_add_epi16(acc, toW);
            
            if(capturedPiece.getType() != EMPTY) {
                int capturedIdx = getFeatureIndex(capturedPiece, toSquare, kingSquare, perspective);
                __m256i* weightCapturedPtr = reinterpret_cast<__m256i*>(featureWeights[capturedIdx].weights.data());
                __m256i capturedW = _mm256_load_si256(weightCapturedPtr + i);
                acc = _mm256_sub_epi16(acc, capturedW);
            }
            
            _mm256_store_si256(accPtr + i, acc);
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
    
    __m256i* accPtr = reinterpret_cast<__m256i*>(accumulator[perspective].values.data());
    __m256i* outPtr = reinterpret_cast<__m256i*>(outputWeights.data());
    
    __m256i sum = _mm256_setzero_si256();
    for(int i = 0; i < HIDDEN_SIZE / 16; ++i) {
        __m256i acc = _mm256_load_si256(accPtr + i);
        __m256i weight = _mm256_load_si256(outPtr + i);
        __m256i prod = _mm256_mulhi_epi16(acc, weight);
        sum = _mm256_add_epi32(sum, _mm256_unpackhi_epi16(prod, prod));
        sum = _mm256_add_epi32(sum, _mm256_unpacklo_epi16(prod, prod));
    }
    
    alignas(32) int32_t temp[8];
    _mm256_store_si256(reinterpret_cast<__m256i*>(temp), sum);
    
    int32_t finalSum = temp[0] + temp[1] + temp[2] + temp[3] + 
                      temp[4] + temp[5] + temp[6] + temp[7];
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
    
    __m256i* accPtr = reinterpret_cast<__m256i*>(accumulator[perspective].values.data());
    for(int y = 0; y < 8; ++y) {
        for(int x = 0; x < 8; ++x) {
            Piece piece = board.getPiece(x, y);
            if(piece.getType() != EMPTY) {
                int square = y * 8 + x;
                int featureIndex = getFeatureIndex(piece, square, kingSquare, perspective);
                
                __m256i* weightPtr = reinterpret_cast<__m256i*>(featureWeights[featureIndex].weights.data());
                for(int i = 0; i < HIDDEN_SIZE / 16; ++i) {
                    __m256i acc = _mm256_load_si256(accPtr + i);
                    __m256i weight = _mm256_load_si256(weightPtr + i);
                    acc = _mm256_add_epi16(acc, weight);
                    _mm256_store_si256(accPtr + i, acc);
                }
            }
        }
    }
    
    applyBatchActivation(accumulator[perspective].values.data(), HIDDEN_SIZE);
    accumulator[perspective].computed = true;
}

void NNUE::applyBatchActivation(int16_t* values, size_t size) {
    __m256i* ptr = reinterpret_cast<__m256i*>(values);
    for(size_t i = 0; i < size / 16; ++i) {
        __m256i val = _mm256_load_si256(ptr + i);
        __m256i zero = _mm256_setzero_si256();
        val = _mm256_max_epi16(_mm256_min_epi16(val, _mm256_set1_epi16(32767)), zero);
        _mm256_store_si256(ptr + i, val);
    }
}

__m256i* NNUE::getSIMDBuffer() {
    return reinterpret_cast<__m256i*>(simdBuffer.get());
} 