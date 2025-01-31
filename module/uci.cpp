#include "../include/uci.h"
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>

UCI::UCI() : running(true), engine(6) {}

void UCI::start() {
    std::string line;
    while(running && std::getline(std::cin, line)) {
        processCommand(line);
    }
}

void UCI::processCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string token;
    iss >> token;
    
    if(token == "uci") {
        std::cout << "id name DeepSquare" << std::endl;
        std::cout << "id author LabWorkShift" << std::endl;
        std::cout << "option name Hash type spin default 128 min 1 max 32768" << std::endl;
        std::cout << "option name Threads type spin default 1 min 1 max 512" << std::endl;
        std::cout << "option name MultiPV type spin default 1 min 1 max 500" << std::endl;
        std::cout << "option name Skill Level type spin default 20 min 0 max 20" << std::endl;
        std::cout << "option name Ponder type check default false" << std::endl;
        std::cout << "uciok" << std::endl;
    }
    else if(token == "debug") {
        iss >> token;
        debugMode = (token == "on");
    }
    else if(token == "isready") {
        std::cout << "readyok" << std::endl;
    }
    else if(token == "setoption") {
        setOption(cmd);
    }
    else if(token == "register") {
        std::cout << "registration ok" << std::endl;
    }
    else if(token == "ucinewgame") {
        board = Board();
        engine.clearTables();
    }
    else if(token == "position") {
        position(cmd);
    }
    else if(token == "go") {
        go(cmd);
    }
    else if(token == "stop") {
        engine.stopSearching();
    }
    else if(token == "ponderhit") {
        // Handle ponderhit
    }
    else if(token == "quit") {
        running = false;
    }
}

void UCI::position(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string token;
    iss >> token; // "position"
    iss >> token;
    
    if(token == "startpos") {
        board = Board();
        iss >> token; // Possibly "moves"
    }
    else if(token == "fen") {
        std::string fen;
        for(int i = 0; i < 6; i++) {
            std::string part;
            iss >> part;
            fen += part + " ";
        }
        board.setFromFEN(fen);
        iss >> token; // Possibly "moves"
    }
    
    if(token == "moves") {
        std::string move;
        while(iss >> move) {
            if(move.length() >= 4) {
                int fromX = move[0] - 'a';
                int fromY = move[1] - '1';
                int toX = move[2] - 'a';
                int toY = move[3] - '1';
                
                // Handle promotion
                PieceType promotionPiece = QUEEN;
                if(move.length() == 5) {
                    switch(move[4]) {
                        case 'n': promotionPiece = KNIGHT; break;
                        case 'b': promotionPiece = BISHOP; break;
                        case 'r': promotionPiece = ROOK; break;
                        case 'q': promotionPiece = QUEEN; break;
                    }
                }
                
                board.makeMove(fromX, fromY, toX, toY, promotionPiece);
            }
        }
    }
}

void UCI::go(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string token;
    iss >> token; // "go"
    
    int wtime = -1, btime = -1, winc = 0, binc = 0;
    int movestogo = 0, depth = -1, nodes = -1, mate = -1;
    int movetime = -1;
    bool infinite = false, ponder = false;
    
    while(iss >> token) {
        if(token == "searchmoves") {
            // Handle searchmoves
            while(iss >> token) {
                if(token == "ponder" || token == "wtime" || token == "btime" || 
                   token == "winc" || token == "binc" || token == "movestogo" ||
                   token == "depth" || token == "nodes" || token == "mate" ||
                   token == "movetime" || token == "infinite") {
                    break;
                }
                // Add move to search moves list
            }
        }
        else if(token == "ponder") ponder = true;
        else if(token == "wtime") iss >> wtime;
        else if(token == "btime") iss >> btime;
        else if(token == "winc") iss >> winc;
        else if(token == "binc") iss >> binc;
        else if(token == "movestogo") iss >> movestogo;
        else if(token == "depth") iss >> depth;
        else if(token == "nodes") iss >> nodes;
        else if(token == "mate") iss >> mate;
        else if(token == "movetime") iss >> movetime;
        else if(token == "infinite") infinite = true;
    }
    
    // Set search parameters
    engine.setSearchParams(depth, movetime, wtime, btime, winc, binc);
    
    // Start search in a separate thread
    std::thread searchThread([this]() {
        Move bestMove = engine.getBestMove(board);
        
        // Output best move
        std::cout << "bestmove " 
                  << static_cast<char>('a' + bestMove.fromX)
                  << static_cast<char>('1' + bestMove.fromY)
                  << static_cast<char>('a' + bestMove.toX)
                  << static_cast<char>('1' + bestMove.toY);
        
        // Add promotion piece if necessary
        if(bestMove.promotion != EMPTY) {
            char promotionChar = 'q';
            switch(bestMove.promotion) {
                case KNIGHT: promotionChar = 'n'; break;
                case BISHOP: promotionChar = 'b'; break;
                case ROOK: promotionChar = 'r'; break;
                case QUEEN: promotionChar = 'q'; break;
            }
            std::cout << promotionChar;
        }
        
        std::cout << std::endl;
    });
    
    searchThread.detach();
}

void UCI::setOption(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string token, name, value;
    
    iss >> token; // "setoption"
    iss >> token; // "name"
    
    while(iss >> token && token != "value") {
        name += token + " ";
    }
    name = name.substr(0, name.length() - 1); // Remove trailing space
    
    while(iss >> token) {
        value += token + " ";
    }
    if(!value.empty()) {
        value = value.substr(0, value.length() - 1); // Remove trailing space
    }
    
    // Handle options
    if(name == "Hash") {
        int hashSize = std::stoi(value);
        engine.setHashSize(hashSize);
    }
    else if(name == "Threads") {
        int threads = std::stoi(value);
        engine.setThreadCount(threads);
    }
    else if(name == "MultiPV") {
        int multiPV = std::stoi(value);
        engine.setMultiPV(multiPV);
    }
    else if(name == "Skill Level") {
        int skillLevel = std::stoi(value);
        engine.setSkillLevel(skillLevel);
    }
    else if(name == "Ponder") {
        bool ponderEnabled = (value == "true");
        engine.setPonder(ponderEnabled);
    }
}