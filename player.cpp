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

    if (!board->hasMoves(side))
        return nullptr;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Move move(x, y);
            if (board->checkMove(&move, side)) {
                board->doMove(&move, side);
                return new Move(x, y);
            }
        }
    }

    return nullptr;
}
