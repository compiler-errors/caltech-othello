#include "board.h"
#include <iostream>
using std::cerr;

//Functions thanks to:
//http://www.gamedev.net/topic/646988-generating-moves-in-reversi/
constexpr uint64_t NORTH(uint64_t x) { return (x << 8); }
constexpr uint64_t SOUTH(uint64_t x) { return (x >> 8); }
constexpr uint64_t EAST(uint64_t x) { return ((x & 0xfefefefefefefefeull) >> 1); }
constexpr uint64_t WEST(uint64_t x) { return ((x & 0x7f7f7f7f7f7f7f7full) << 1); }
constexpr uint64_t NOREAST(uint64_t x) { return NORTH(EAST(x)); }
constexpr uint64_t SOUEAST(uint64_t x) { return SOUTH(EAST(x)); }
constexpr uint64_t NORWEST(uint64_t x) { return NORTH(WEST(x)); }
constexpr uint64_t SOUWEST(uint64_t x) { return SOUTH(WEST(x)); }

/*
 * Generates possible moves for pieces of the bitboard 'own' in the direction
 * given by the function pointer shift().
 * My hope here is that GCC will inline the function pointer and essentially
 * unroll all 8 calls to generateMove() for each direction
 */
inline uint64_t generateMove(uint64_t(*shift)(uint64_t), uint64_t own, uint64_t other) {
    uint64_t empty = ~(own | other); //Bitboard with '1' at empty pieces.
    //Possible stores the possible (well, adjacent-to-possible) moves.
    uint64_t possible = shift(own) & other; //Move the piece 1 step
    possible |= (shift(possible) & other); //Move the piece 2 steps
    possible |= (shift(possible) & other); //3
    possible |= (shift(possible) & other); //4
    possible |= (shift(possible) & other); //5
    possible |= (shift(possible) & other); //6
    //We don't need to go on any longer, since we would be running off the board
    return shift(possible) & empty; //now which adjacent pieces are empty?
}

/* Returns a bitboard with a single 1 at position (x, y) */
constexpr uint64_t getSinglePosition(int x, int y) {
    return (0x8000000000000000ull >> (8 * y)) >> x;
}

/*
 * Returns a copy of this board.
 */
Board *Board::copy() {
    Board *newBoard = new Board();
    newBoard->black = black;
    newBoard->white = white;
    newBoard->black_moves = black_moves;
    newBoard->white_moves = white_moves;
    return newBoard;
}

Board* Board::copyDoMove(Move* m, Side side) {
    Board* newBoard = copy();
    newBoard->doMove(m, side);
    return newBoard;
}

bool Board::occupied(int x, int y) {
    return get(WHITE, x, y) || get(BLACK, x, y);
}

bool Board::get(Side side, int x, int y) {
    return (side == BLACK ? black : white) & getSinglePosition(x, y);
}

void Board::set(Side side, int x, int y) {
    (side == BLACK ? black : white) |= getSinglePosition(x, y);
}

void Board::generateMoves() {
    //Generate black moves first:
    black_moves = generateMove(NORTH, black, white)
                | generateMove(SOUTH, black, white)
                | generateMove(EAST, black, white)
                | generateMove(WEST, black, white)
                | generateMove(NOREAST, black, white)
                | generateMove(SOUEAST, black, white)
                | generateMove(NORWEST, black, white)
                | generateMove(SOUWEST, black, white);

    white_moves = generateMove(NORTH, white, black)
                | generateMove(SOUTH, white, black)
                | generateMove(EAST, white, black)
                | generateMove(WEST, white, black)
                | generateMove(NOREAST, white, black)
                | generateMove(SOUEAST, white, black)
                | generateMove(NORWEST, white, black)
                | generateMove(SOUWEST, white, black);
}

/*
 * Returns true if the game is finished; false otherwise. The game is finished
 * if neither side has a legal move.
 */
bool Board::isDone() {
    return !(hasMoves(BLACK) || hasMoves(WHITE));
}

/*
 * Returns true if there are legal moves for the given side.
 */
bool Board::hasMoves(Side side) {
    return (side == BLACK ? black_moves : white_moves) != 0;
}

/*
 * Returns true if a move is legal for the given side; false otherwise.
 */
bool Board::checkMove(Move *m, Side side) {
    return (getSinglePosition(m->getX(), m->getY()) & (side == BLACK ? black_moves : white_moves)) != 0;
}

/* Carries out a move in a specific direction */
uint64_t Board::doDirection(int x, int y, Side side, uint64_t(*shift)(uint64_t)) {
    uint64_t m = getSinglePosition(x, y);
    uint64_t changed = m;
    uint64_t otherBoard = (side == BLACK ? white : black);
    uint64_t ourBoard = (side == BLACK ? black : white);

    //Keep shifting 'm' until we are no longer over a white piece
    //tracking the change on "changed".
    while ((m = shift(m)) & otherBoard)
        changed |= m;

    //If the next piece in that direction is on our board,
    //then return all the changed pieces.
    if (m & ourBoard) {
        return changed;
    }

    return 0;
}

/*
 * Returns a vector of possible legal moves for the current board state.
 */
vector<Move> Board::getMoves(Side side)
{
    vector<Move> moves_list;
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            Move move(x,y);
            if (checkMove(&move, side))
                moves_list.push_back(move);
        }
    }
    return moves_list;
}

/*
 * Modifies the board to reflect the specified move.
 */
bool Board::doMove(Move* m, Side side) {
    // Passing is only legal if you have no moves.
    if (m == nullptr) return !hasMoves(side);

    int x = m->getX();
    int y = m->getY();

    // Make sure the move is correct.
    if (!checkMove(m, side))
        return false;

    //newBoard is essentially the bitmap of pieces which have changed hands.
    uint64_t newBoard = doDirection(x, y, side, NORTH);
    newBoard |= doDirection(x, y, side, SOUTH);
    newBoard |= doDirection(x, y, side, EAST);
    newBoard |= doDirection(x, y, side, WEST);
    newBoard |= doDirection(x, y, side, NOREAST);
    newBoard |= doDirection(x, y, side, NORWEST);
    newBoard |= doDirection(x, y, side, SOUEAST);
    newBoard |= doDirection(x, y, side, SOUWEST);

    if (newBoard == 0)
        return false;

    //We apply it to the board that receives the new pieces
    (side == BLACK ? black : white) |= newBoard;
    //And then we can do a sanity check and "un-apply" them from the other.
    (side == BLACK ? white : black) &= ~(side == BLACK ? black : white);

    set(side, x, y);
    generateMoves();

    return true;
}

/*
 * Current count of given side's stones.
 */
int Board::count(Side side) {
    return (side == BLACK) ? countBlack() : countWhite();
}

/*
 * Current count of black stones.
 */
int Board::countBlack() {
    uint64_t board = black;

    int count = 0;
    while (board) {
        count += board & 1;
        board >>= 1;
    }

    return count;
}

/*
 * Current count of white stones.
 */
int Board::countWhite() {
    uint64_t board = white;

    int count = 0;
    while (board) {
        count += board & 1;
        board >>= 1;
    }

    return count;
}

/*
 * Determines the number of occupied spaces that are corners (very good, x5),
 * edges (good, x2), adjacent to corners (bad, x(-2)), and diagonal to corners
 * (very bad, x(-5)). These modifiers are applied to compute the final score of
 * a position.
 */
int Board::score(Side side)
{
    // number of occupied corners
    int corn = 0;
    corn += get(side, 0, 0);
    corn += get(side, 0, 7);
    corn += get(side, 7, 0);
    corn += get(side, 7, 7);

    // number of occupied edge spots (not adjacent to corners)
    int edge = 0;
    for (int i = 2; i < 6; i++)
    {
        edge += get(side, 0, i);
        edge += get(side, i, 0);
        edge += get(side, 7, i);
        edge += get(side, i, 7);
    }
    
    // number of occupied spots diagonal to corners
    int d_corn = 0;
    d_corn += get(side, 1, 1);
    d_corn += get(side, 1, 6);
    d_corn += get(side, 6, 1);
    d_corn += get(side, 6, 6);

    // number of occupied spots adjacent to corners
    int adj_corn = 0;
    adj_corn += get(side, 0, 1);
    adj_corn += get(side, 0, 6);
    adj_corn += get(side, 1, 0);
    adj_corn += get(side, 1, 7);
    adj_corn += get(side, 6, 0);
    adj_corn += get(side, 6, 7);
    adj_corn += get(side, 7, 1);
    adj_corn += get(side, 7, 6);

    // multipliers are 4 & 1 for good and 6 & 3 for bad to account for +1x
    // already in count
    int good = 4 * corn + edge;
    int bad = 6 * d_corn + 3 * adj_corn;
    return count(side) + good - bad;
}

/*
 * Sets the board state given an 8x8 char array where 'w' indicates a white
 * piece and 'b' indicates a black piece. Mainly for testing purposes.
 */
void Board::setBoard(char data[]) {
    for (int i = 0; i < 64; i++) {
        if (data[i] == 'b') {
            set(BLACK, i / 8, i % 8);
        } if (data[i] == 'w') {
            set(WHITE, i / 8, i % 8);
        }
    }

    generateMoves();
}
