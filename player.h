#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include "common.h"
#include "board.h"
using namespace std;

class Player {
public:
    Player(Side s);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    Move getBestMove();
    int naiveMinimax(Board* current, Side side, int depth, bool max);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;

public:
    Board *board;
    Side ourSide, opponentSide;
};

#endif
