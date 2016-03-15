#include <cstdio>
#include "common.h"
#include "player.h"
#include "board.h"

// Use this file to test your minimax implementation (2-ply depth, with a
// heuristic of the difference in number of pieces).
int main(int argc, char *argv[]) {

    // Create board with example state. You do not necessarily need to use
    // this, but it's provided for convenience.
    char boardData[64] = {
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', 'w', ' ', ' ',
        'b', ' ', 'w', 'w', 'w', 'w', 'w', ' ',
        'w', 'b', 'w', 'w', 'b', 'w', ' ', ' ',
        ' ', 'b', 'b', 'b', 'w', ' ', 'w', ' ',
        'b', 'b', 'b', 'w', 'w', 'w', ' ', ' ',
        ' ', ' ', 'b', 'w', ' ', ' ', ' ', ' ',
        ' ', ' ', 'b', ' ', ' ', ' ', ' ', ' '
    };
    Board *board = new Board();
    board->setBoard(boardData);

    // Initialize player as the white player, and set testing_minimax flag.
    Player *player = new Player(WHITE);
    player->testingMinimax = true;
    player->board = board;


    /**
     * TODO: Write code to set your player's internal board state to the
     * example state.
     */

    // Get player's move and check if it's right.
    Move *move = player->doMove(NULL, 0);
}
