#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <iostream>
#include "common.h"
#include "board.h"
#include <unordered_map>
using namespace std;

class Player {
public:
    Player(Side s);
    ~Player();

    Move *doMove(Move *opponentsMove, int msLeft);
    Move getBestMove();
    //int naiveMinimax(Board* current, Side side, int depth, bool max, Move& bestMove, int elapsedMoves);
    int negamax(Board *current, Side player, int depth, int a, int b, int elapsed_moves, Move &ret);

    // Flag to tell if the player is running within the test_minimax context
    bool testingMinimax;

public:
    Board *board;
    Side ourSide, opponentSide;
    int elapsed_moves;
    bool finalMode;
};

#endif
