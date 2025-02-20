#ifndef NNUE_H
#define NNUE_H

#include "board.h"
#include "move.h"
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <immintrin.h>

#ifdef _MSC_VER
#pragma warning(disable: 4324)
#endif

class NNUE {
private:
    static constexpr int KING_BUCKETS = 64;
    static constexpr int INPUTS_PER_KING = 640;
    static constexpr int INPUT_SIZE = KING_BUCKETS * INPUTS_PER_KING;
    static constexpr int HIDDEN_SIZE = 256;
    static constexpr int OUTPUT_SIZE = 1;
    
    struct alignas(32) AccumulatorEntry {
        std::array<int16_t, HIDDEN_SIZE> values;
        int kingSquare;
        bool computed;
        std::array<uint8_t, 27> padding;
    };
    
    struct alignas(32) LayerWeights {
        std::array<int16_t, HIDDEN_SIZE> weights;
        int16_t bias;
        std::array<uint8_t, 30> padding;
    };
    
    std::vector<LayerWeights> featureWeights;
    std::array<int16_t, HIDDEN_SIZE> outputWeights;
    int16_t outputBias;
    
    std::array<AccumulatorEntry, 2> accumulator;
    std::array<AccumulatorEntry, 32> accumulatorStack;
    int accumulatorStackSize;
    
    std::unique_ptr<uint8_t[]> simdBuffer;
    
public:
    NNUE();
    ~NNUE();
    
    NNUE(const NNUE& other);
    NNUE& operator=(const NNUE& other);
    NNUE(NNUE&& other) noexcept;
    NNUE& operator=(NNUE&& other) noexcept;
    
    void loadWeights(const std::string& filename);
    void refreshAccumulator(const Board& board);
    void updateAccumulator(const Board& board, const Move& move);
    int evaluate(const Board& board, bool perspective);
    
    void pushAccumulator();
    void popAccumulator();
    
private:
    int16_t clamp(int32_t x);
    int getFeatureIndex(const Piece& piece, int square, int kingSquare, bool perspective) const;
    void initializeAccumulator(const Board& board, bool perspective);
    void applyBatchActivation(int16_t* values, size_t size);
    __m256i* getSIMDBuffer();
};

#endif