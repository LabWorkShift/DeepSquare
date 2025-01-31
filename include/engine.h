#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "move.h"
#include "nnue.h"
#include <vector>
#include <atomic>
#include <string>

class Engine {
private:
    struct SearchInfo {
        uint64_t nodes;
        int depth;
        std::vector<Move> pv;
        int score;
    };

    int searchDepth;
    int moveTime;
    int timeWhite;
    int timeBlack;
    int incrementWhite;
    int incrementBlack;
    std::atomic<bool> stopSearch;
    NNUE evaluator;
    
    // New members for UCI options
    int hashSize;
    int threadCount;
    int multiPV;
    int skillLevel;
    bool ponderEnabled;
    bool debugMode;
    
public:
    Engine(int depth = 4);
    Move getBestMove(const Board& board);
    void setSearchParams(int depth, int movetime, int wtime, int btime, int winc, int binc);
    void stopSearching() { stopSearch = true; }
    bool isStopRequested() const { return stopSearch; }
    
    // New UCI option methods
    void clearTables();
    void setHashSize(int size) { hashSize = size; }
    void setThreadCount(int count) { threadCount = count; }
    void setMultiPV(int mpv) { multiPV = mpv; }
    void setSkillLevel(int level) { skillLevel = level; }
    void setPonder(bool enable) { ponderEnabled = enable; }
    void setDebugMode(bool enable) { debugMode = enable; }
    
private:
    int minimax(Board& board, int depth, int alpha, int beta, bool maximizing, SearchInfo& info);
    std::vector<Move> generateAllMoves(const Board& board, bool forWhite);
    bool isTimeUp();
    void orderMoves(std::vector<Move>& moves, const Board& board);
    uint64_t perft(Board& board, int depth);
    std::string moveToString(const Move& move);
};

#endif 