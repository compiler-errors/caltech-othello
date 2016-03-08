#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish
 * within 30 seconds.
 */
Player::Player(Side s) {
    testingMinimax = false;
    board = new Board();
    side = s;
    opponentSide = (s == BLACK ? WHITE : BLACK);
}

/*
 * Destructor for the player.
 */
Player::~Player() {
}

/*
 * Compute the next move given the opponent's last move. Your AI is
 * expected to keep track of the board on its own. If this is the first move,
 * or if the opponent passed on the last move, then opponentsMove will be NULL.
 *
 * msLeft represents the time your AI has left for the total game, in
 * milliseconds. doMove() must take no longer than msLeft, or your AI will
 * be disqualified! An msLeft value of -1 indicates no time limit.
 *
 * The move returned must be legal; if there are no valid moves for your side,
 * return NULL.
 */
Move *Player::doMove(Move *opponentsMove, int msLeft) {
    if (opponentsMove != nullptr) {
        board->doMove(opponentsMove, opponentSide);
    }

    vector<Move> moves = board->getMoves(side);
    if (moves.size() == 0)
        return nullptr;

    Move best = testMoves(moves);
    board->doMove(&best, side);
    Move *move = new Move(best.getX(), best.getY());
    return move;
}

/*
 * Tests all moves legal for present state and returns the optimal one.
 */
Move Player::testMoves(vector<Move> moves)
{
    int best = 0;
    vector<int> scores;

    for (unsigned int i = 0; i < moves.size(); i++)
    {
        Board *copy = board->copy();
        copy->doMove(&moves[i], side);

        //vector

        scores.push_back(copy->count(side));
        delete copy;
    }

    for (unsigned int i = 0; i < scores.size(); i++)
    {
            if (scores[i] > scores[best])
            best = i;
    }

    return moves[best];
}
