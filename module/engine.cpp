#include "../include/engine.h"
#include "../include/evaluation.h"
#include <algorithm>
#include <limits>
#include <chrono>
#include <iostream>

Engine::Engine(int depth) : searchDepth(depth), stopSearch(false), 
    hashSize(128), threadCount(1), multiPV(1), skillLevel(20), 
    ponderEnabled(false), debugMode(false) {}

void Engine::clearTables() {
    // Reset transposition table if implemented
    // Reset evaluation cache if implemented
    // Reset any other tables or caches
    stopSearch = false;
    
    // Reset NNUE tables if necessary
    evaluator = NNUE(); // Reinitialize NNUE
}

Move Engine::getBestMove(const Board& board) {
    stopSearch = false;
    SearchInfo info = {0};
    std::vector<Move> moves = generateAllMoves(board, board.isWhiteToMove());
    
    if(moves.empty()) {
        return Move();
    }
    
    // Order moves for better pruning
    orderMoves(moves, board);
    
    // Start iterative deepening
    for(int currentDepth = 1; currentDepth <= searchDepth; currentDepth++) {
        if(isTimeUp()) break;
        
        int alpha = -std::numeric_limits<int>::max();
        int beta = std::numeric_limits<int>::max();
        
        Board tempBoard = board;
        info.depth = currentDepth;
        
        for(const Move& move : moves) {
            if(isTimeUp()) break;
            
            tempBoard = board;
            tempBoard.makeMove(move.fromX, move.fromY, move.toX, move.toY, move.promotion);
            
            int score = -minimax(tempBoard, currentDepth - 1, -beta, -alpha, false, info);
            
            if(score > alpha) {
                alpha = score;
                info.pv.clear();
                info.pv.push_back(move);
            }
        }
        
        // Output search info
        if(debugMode) {
            std::cout << "info depth " << currentDepth 
                      << " score cp " << alpha 
                      << " nodes " << info.nodes 
                      << " pv " << moveToString(info.pv[0]) << std::endl;
        }
    }
    
    return !info.pv.empty() ? info.pv[0] : moves[0];
}

void Engine::setSearchParams(int depth, int movetime, int wtime, int btime, int winc, int binc) {
    searchDepth = depth > 0 ? depth : searchDepth;
    moveTime = movetime;
    timeWhite = wtime;
    timeBlack = btime;
    incrementWhite = winc;
    incrementBlack = binc;
}

bool Engine::isTimeUp() {
    // Implement time management
    return stopSearch;
}

void Engine::orderMoves(std::vector<Move>& moves, const Board& board) {
    // Score moves for ordering
    for(Move& move : moves) {
        int score = 0;
        
        // MVV-LVA scoring
        Piece captured = board.getPiece(move.toX, move.toY);
        if(captured.getType() != EMPTY) {
            score += 10 * captured.getValue();
        }
        
        // Promotion scoring
        if(move.promotion != EMPTY) {
            score += 1000;
        }
        
        move.score = score;
    }
    
    // Sort moves by score
    std::sort(moves.rbegin(), moves.rend());
}

int Engine::minimax(Board& board, int depth, int alpha, int beta, bool maximizing, SearchInfo& info) {
    if(isTimeUp()) return 0;
    
    info.nodes++;
    
    if(depth == 0) {
        return evaluator.evaluate(board, maximizing);
    }
    
    std::vector<Move> moves = generateAllMoves(board, maximizing);
    if(moves.empty()) {
        if(board.isCheck()) {
            return maximizing ? -20000 : 20000; // Checkmate
        }
        return 0; // Stalemate
    }
    
    orderMoves(moves, board);
    
    for(const Move& move : moves) {
        board.makeMove(move.fromX, move.fromY, move.toX, move.toY, move.promotion);
        int score = -minimax(board, depth - 1, -beta, -alpha, !maximizing, info);
        board = board; // Undo move
        
        if(maximizing) {
            alpha = std::max(alpha, score);
        } else {
            beta = std::min(beta, score);
        }
        
        if(beta <= alpha) {
            break;
        }
    }
    
    return maximizing ? alpha : beta;
}

std::vector<Move> Engine::generateAllMoves(const Board& board, bool forWhite) {
    std::vector<Move> moves;
    
    for(int fromY = 0; fromY < 8; fromY++) {
        for(int fromX = 0; fromX < 8; fromX++) {
            Piece piece = board.getPiece(fromX, fromY);
            if(piece.getType() != EMPTY && (piece.getColor() == WHITE) == forWhite) {
                auto legalMoves = board.getLegalMoves(fromX, fromY);
                for(const auto& to : legalMoves) {
                    moves.emplace_back(fromX, fromY, to.first, to.second);
                }
            }
        }
    }
    
    return moves;
}

uint64_t Engine::perft(Board& board, int depth) {
    if(depth == 0) return 1;
    
    uint64_t nodes = 0;
    std::vector<Move> moves = generateAllMoves(board, board.isWhiteToMove());
    
    for(const Move& move : moves) {
        Board tempBoard = board;
        if(tempBoard.makeMove(move.fromX, move.fromY, move.toX, move.toY)) {
            nodes += perft(tempBoard, depth - 1);
        }
    }
    
    return nodes;
}

std::string Engine::moveToString(const Move& move) {
    std::string result;
    result += static_cast<char>('a' + move.fromX);
    result += static_cast<char>('1' + move.fromY);
    result += static_cast<char>('a' + move.toX);
    result += static_cast<char>('1' + move.toY);
    return result;
}