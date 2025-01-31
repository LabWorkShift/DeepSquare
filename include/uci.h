#ifndef UCI_H
#define UCI_H

#include "engine.h"
#include "board.h"
#include <string>

class UCI {
private:
    bool running;
    bool debugMode;
    Engine engine;
    Board board;
    
public:
    UCI();
    void start();
    void processCommand(const std::string& cmd);
    void position(const std::string& cmd);
    void go(const std::string& cmd);
    void setOption(const std::string& cmd);
    std::string moveToString(const Move& move);
    Move stringToMove(const std::string& moveStr);
};

#endif 