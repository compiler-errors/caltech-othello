#ifndef __BOARD_H__
#define __BOARD_H__

#include "common.h"
using namespace std;

class Board {
private:
    uint64_t black, white;
    uint64_t black_moves, white_moves;

    bool occupied(int x, int y);
    bool get(Side side, int x, int y);
    void set(Side side, int x, int y);
    void generateMoves();
    uint64_t doDirection(int x, int y, Side side, uint64_t(*shift)(uint64_t));

public:
    // The board is initialized to the bitmaps that signify the starting positions.
    Board() : black(34628173824), white(68853694464) { generateMoves(); };
    ~Board() = default;
    Board* copy();
    Board* copyDoMove(Move* m, Side side);

    bool isDone();
    bool hasMoves(Side side);
    bool checkMove(Move *m, Side side);
    vector<Move> getMoves(Side side);
    bool doMove(Move *m, Side side);
    int count(Side side);
    int countBlack();
    int countWhite();
    int score(Side side);

    void setBoard(char data[]);
};

#endif
