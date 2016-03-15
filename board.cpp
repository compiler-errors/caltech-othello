#include "board.h"
#include "constants.h"

#define COMBINE(a, b) ((a + b) == 0 ? (0.0) : (100*(a-b)*1.0/(a+b)))

/*
 * Generates possible moves for pieces of the bitboard 'own' in the direction
 * given by the function pointer shift
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

#define IS_STABLE(pos) (!(pos) || ((pos) & stablePieces))

inline uint64_t Board::generateStablePieces(Side side) {
    uint64_t ourBoard = (side == WHITE ? white : black);
    if (!(ourBoard & all_corners)) //Stable pieces cannot exist w/o corners.
        return 0ull;

    //we begin by finding the row,column,diagonal-locked stable pieces
    uint64_t board = white | black;
    uint64_t locked_rows = 0, locked_columns = 0;
    for (int i = 0; i < 8; i++) {
        if ((board & rows[i]) == rows[i])
            locked_rows |= rows[i];
        if ((board & columns[i]) == columns[i])
            locked_columns |= columns[i];
    }

    uint64_t locked_diag = 0, locked_antidiag = 0;
    for (int i = 0; i < 15; i++) {
        if ((board & diagonals[i]) == diagonals[i])
            locked_diag |= diagonals[i];
        if ((board & anti_diagonals[i]) == anti_diagonals[i])
            locked_antidiag |= anti_diagonals[i];
    }

    uint64_t stablePieces = (side == WHITE ? white_stables : black_stables);
    stablePieces |= ourBoard & locked_rows & locked_columns & locked_diag & locked_antidiag;
    stablePieces |= (ourBoard & all_corners);

    for (int i = 0; i < 8; i++) {
        bool newstable = false;

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                uint64_t pos = getSinglePosition(x, y);
                //cerr << x << ", " << y << " and our board is " << ((pos & ourBoard) == 0 ? "FALSE " : "TRUE") << "\n";
                if ((pos & stablePieces) || !(pos & ourBoard))
                    continue;

                uint64_t n = NORTH(pos), s = SOUTH(pos),
                         e = EAST(pos), w = WEST(pos),
                         ne = NOREAST(pos), se = SOUEAST(pos),
                         nw = NORWEST(pos), sw = SOUWEST(pos);

                if ((IS_STABLE(n) || IS_STABLE(s) || (pos & locked_columns)) &&
                    (IS_STABLE(e) || IS_STABLE(w) || (pos & locked_rows)) &&
                    (IS_STABLE(ne) || IS_STABLE(sw) || (pos & locked_diag)) &&
                    (IS_STABLE(nw) || IS_STABLE(se) || (pos & locked_antidiag))) {
                    stablePieces |= pos;
                    newstable = true;
                }
            }
        }

        if (!newstable)
            break;
    }

    return stablePieces;
}

/*
 * Returns a copy of this board.
 */
Board *Board::copy() {
    return new Board(*this);
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

    black_stables = generateStablePieces(BLACK);
    white_stables = generateStablePieces(WHITE);
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
    uint64_t ourBoard = (side == BLACK ? black : white);
    uint64_t otherBoard = (side == BLACK ? white : black);

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
        count += (board & 1);
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
        count += (board & 1);
        board >>= 1;
    }

    return count;
}

/*
 * Returns the strategic value of a board position.
 */
int Board::score(Side side, int elapsedMoves)
{
    if (isDone()) {
        if (count(side) > count(OPPOSITE(side))) {
            return INT_MAX - 1;
        } else
            return -(INT_MAX - 1);
    }

    //It is significantly easier to calculate the score as if it were always white
    //and then simply flipping the score if we are instead calculating for black.
    int whiteCoins = 0, blackCoins = 0;
    int whiteMoves = 0, blackMoves = 0;
    int whiteUtility = 0, blackUtility = 0;
    int whiteStables = 0, blackStables = 0;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            uint64_t pos = getSinglePosition(x, y);
            if (white & pos) {
                whiteCoins++;
                whiteUtility += utilityMatrix[x][y];
                if (pos & white_stables)
                    whiteStables++;
            } else if (black & pos) {
                blackCoins++;
                blackUtility += utilityMatrix[x][y];
                if (pos & black_stables)
                    blackStables++;
            } else {
                if (white_moves & pos)
                    whiteMoves++;
                if (black_moves & pos) // NOT 'else', since they can both have it.
                    blackMoves++;
            }
        }
    }

    float coinParity = COMBINE(whiteCoins, blackCoins);
    float moveParity = COMBINE(whiteMoves, blackMoves);
    float utilityParity = (whiteUtility - blackUtility)*10;
    float stableParity = COMBINE(whiteStables, blackStables);
    float finalScore = 0;

    if (elapsedMoves < 20) {
        finalScore = 100 * (utilityParity * 0.55 + moveParity * 0.5 + coinParity * 0.05) / (0.55 + 0.5 + 0.05);
    } else if (elapsedMoves < 30) {
        finalScore = 100 * (utilityParity * 0.55 + moveParity * 0.5 - coinParity * 0.05 + stableParity * 0.3) / (1.3);
    } else if (elapsedMoves < 40) {
        finalScore = 100 * (utilityParity * 0.55 + moveParity * 0.5 - coinParity * 0.05 + stableParity * 0.4) / (1.4);
    } else {
        finalScore = 100 * (utilityParity * 0.4 + moveParity * 0.2 + coinParity * 0.15 + stableParity * 0.5) / (0.4 + 0.5 + 0.15 + 0.8);
    }

    return (int) (side == WHITE ? finalScore : -finalScore);
}

void Board::printBoard() {
        cerr << "board is: \n";
        for (int y = 0; y < 8; y++) {
        cerr << "-----------------\n";
            for (int x = 0; x < 8; x++) {
                cerr << "|";
                uint64_t pos = getSinglePosition(x, y);
                if (pos & white_stables) { //TODO: make prettier
                    cerr << "W";
                } else if (pos & black_stables) {
                    cerr << "B";
                } else if (get(WHITE, x, y)) {
                    cerr << "w";
                } else if (get(BLACK, x, y)) {
                    cerr << "b";
                } else if (black_moves & getSinglePosition(x, y)) {
                    cerr << "^";
                } else if (white_moves & getSinglePosition(x, y)) {
                    cerr << "*";
                } else
                    cerr << " ";
            }
            cerr << "|\n";
        }
        cerr << "-----------------\n\n";
}

/*
 * Sets the board state given an 8x8 char array where 'w' indicates a white
 * piece and 'b' indicates a black piece. Mainly for testing purposes.
 */
void Board::setBoard(char data[]) {
    black = 0;
    white = 0;
    black_moves = 0;
    white_moves = 0;


    for (int i = 0; i < 64; i++) {
        if (data[i] == 'b') {
            set(BLACK, i % 8, i / 8);
        } if (data[i] == 'w') {
            set(WHITE, i % 8, i / 8);
        }
    }

    generateMoves();
}
