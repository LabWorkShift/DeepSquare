#ifndef NNUE_H
#define NNUE_H

#include "board.h"
#include "move.h"
#include <array>
#include <vector>
#include <string>

class NNUE {
private:
    static constexpr int INPUT_SIZE = 768;
    static constexpr int HIDDEN_SIZE = 256;
    static constexpr int OUTPUT_SIZE = 1;
    
    struct AccumulatorEntry {
        std::array<int16_t, HIDDEN_SIZE> values;
        bool computed;
    };
    
    std::array<std::array<int16_t, HIDDEN_SIZE>, INPUT_SIZE> weights1;
    std::array<int16_t, HIDDEN_SIZE> biases1;
    std::array<int16_t, OUTPUT_SIZE> weights2;
    int16_t bias2;
    
    AccumulatorEntry accumulator[2];
    
public:
    NNUE();
    void loadWeights(const std::string& filename);
    void refreshAccumulator(const Board& board);
    void updateAccumulator(const Board& board, const Move& move);
    int evaluate(const Board& board, bool perspective);
    
private:
    int16_t clamp(int32_t x);
    int getFeatureIndex(const Piece& piece, int square, bool perspective) const;
    void initializeAccumulator(const Board& board, bool perspective);
};

#endif