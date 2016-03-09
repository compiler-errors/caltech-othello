#include "player.h"

/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish
 * within 30 seconds.
 */
Player::Player(Side s) {
    testingMinimax = false;
    board = new Board();
    ourSide = s;
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
        cerr << "Opponent did " << int(opponentsMove->x) << ", " << int(opponentsMove->y) << "\n";
    } else {
        cerr << "Opponent passed\n";
    }

    Move best = getBestMove();
    board->doMove(&best, ourSide);
    Move *move = new Move(best);
    //cerr << "Our score is " << board->score(side) << "\n";
    //cerr << "White is " << board->count(WHITE) << ", Black is " << board->count(BLACK) << "\n";


    return move;
}

/*
 * Tests all moves legal for present state and returns the optimal one.
 */
Move Player::getBestMove()
{
    board->printBoard();
    Move bestMove(-1, -1);
    if (testingMinimax) {
        naiveMinimax(board, ourSide, 2, true, bestMove);
    } else {
        naiveMinimax(board, ourSide, 6, true, bestMove);
    }
    return bestMove;
}



int Player::naiveMinimax(Board* current, Side player, int depth, bool max, Move& returnMove) {
    if (depth == 0 || current->isDone()) {
        if (testingMinimax)
            return current->count(ourSide) - current->count(opponentSide);
        else
            return current->score(ourSide);
    }

    Move dummy(-1, -1);

    if (!current->hasMoves(player)) {
        return naiveMinimax(current, OPPOSITE(player), depth - 1, !max, dummy);
    }

    if (max) {
        Move bestMove(-1, -1);
        int bestScore = INT_MIN;
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                Move move(x, y);
                if (!current->checkMove(&move, player))
                    continue;

                Board* copy = current->copyDoMove(&move, player);
                int score = naiveMinimax(copy, OPPOSITE(player), depth - 1, !max, dummy);
                delete copy;

                if (score > bestScore) {
                    bestMove = move;
                    bestScore = score;
                }
            }
        }

        returnMove = bestMove;
        return bestScore;
    } else { //minimizing
        Move worstMove(-1, -1);
        int worstScore = INT_MAX;
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                Move move(x, y);
                if (!current->checkMove(&move, player))
                    continue;

                Board* copy = current->copyDoMove(&move, player);
                int score = naiveMinimax(copy, OPPOSITE(player), depth - 1, !max, dummy);
                delete copy;

                if (score < worstScore) {
                    worstScore = score;
                    worstMove = move;
                }
            }
        }

        returnMove = worstMove;
        return worstScore;
    }
}
