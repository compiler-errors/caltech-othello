#include "player.h"
#include <algorithm>
#include <chrono>

// ------------------------------------------------------------ //
unsigned long currentTimeMillis() {
    return std::chrono::system_clock::now().time_since_epoch() /
    std::chrono::milliseconds(1);
}

unsigned long timeUpTime = 0;

void setDuration(long millis) {
    timeUpTime = currentTimeMillis() + millis;
}

bool outOfTime() {
    return currentTimeMillis() > timeUpTime;
}
// ------------------------------------------------------------ //
unsigned history_table[8][8];

bool cmp(const Move& a, const Move& b) {
    return history_table[a.x][a.y] > history_table[b.x][b.y];
}
// ------------------------------------------------------------ //
enum Exactness { LOWER, UPPER, EXACT };

struct Bucket {
    int popularity;
    uint64_t white, black;

    uint8_t depth;
    Move best_move;
    int value;
    Exactness exactness;

    Bucket() {
        popularity = -100;
        white = black = 0ull;
        depth = 0;
        best_move = Move(0, 0);
        value = 0;
    }
};

inline int getHash(Board* board) {
    hash<uint64_t> h;
    return (h(board->white) + h(board->black)) % 1000000;
}

Bucket black_hash_table[1000000];
Bucket white_hash_table[1000000];

Bucket* try_retrieve(Board* board, Side side) {
    Bucket* bucket = (side == BLACK ? black_hash_table : white_hash_table) + getHash(board);

    if (bucket->white == board->white && bucket->black == board->black) {
        //hit++;
        bucket->popularity++;
        return bucket;
    } else {
        //miss++;
        bucket->popularity--;
        return nullptr;
    }
}

void try_save(Board* board, int score, Move move, int alpha, int beta, int depth, Side side) {
    Bucket& bucket = (side == BLACK ? black_hash_table : white_hash_table)[getHash(board)];
    Exactness flag;

    if (score <= alpha) {
        flag = UPPER;
    } else if (score >= beta) {
        flag = LOWER;
    } else {
        flag = EXACT;
    }

    if ((bucket.white != board->white || bucket.black != board->black) &&
        (bucket.white != 0 && bucket.black != 0)) {
        if (depth > bucket.depth)
            bucket.popularity -= 1000;
        else if (depth == bucket.depth)
            return;
    } else if (bucket.white == board->white && bucket.black == board->black) {
        if (bucket.depth > depth)
            return;
        if (bucket.exactness != flag && bucket.exactness != EXACT)
            return;
        else {
            if ((flag == UPPER && bucket.value > score) ||
                (flag == LOWER && bucket.value < score) ||
                (flag == EXACT))
                return;
        }
    }

    if (bucket.popularity > -100)
        return;

    //bo++;

    bucket.white = board->white;
    bucket.black = board->black;
    bucket.exactness = flag;
    bucket.value = score;
    bucket.best_move = move;
    bucket.depth = depth;
    bucket.popularity = 500;
}
// ------------------------------------------------------------ //


/*
 * Constructor for the player; initialize everything here. The side your AI is
 * on (BLACK or WHITE) is passed in as "side". The constructor must finish
 * within 30 seconds.
 */
Player::Player(Side s) {
    //Let's clear out the history table.
    for (int i = 0; i < 64; i++)
        history_table[i / 8][i % 8] = 0;

    testingMinimax = false;
    board = new Board();
    ourSide = s;
    opponentSide = (OPPOSITE(s));
    elapsed_moves = 0;
    finalMode = false;
}

/*
 * Destructor for the player.
 */
Player::~Player() {
    delete board;
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
    if (msLeft > 0) {
        if (finalMode) {
            //cerr << "Final mode.\n";
            setDuration(msLeft / 4);
        } else {
            int msThisMove = (msLeft) / (64 - board->countBlack() - board->countWhite());
            setDuration(msThisMove);
            //cerr << "We have " << msThisMove << " millis to spare\n";
        }
    } else {
        setDuration(100000);
    }
    elapsed_moves++;

    if (opponentsMove != nullptr) {
        board->doMove(opponentsMove, opponentSide);
    //    cerr << "Opponent did " << int(opponentsMove->x) << ", " << int(opponentsMove->y) << "\n";
    } else {
    //    cerr << "Opponent passed\n";
    }

    Move best = getBestMove();
    board->doMove(&best, ourSide);
    Move *move = new Move(best.x, best.y);

    elapsed_moves++;
    return move;
}

/*
 * Tests all moves legal for present state and returns the optimal one.
 */
Move Player::getBestMove()
{
    if (!finalMode && (elapsed_moves >= 44 || board->countBlack() + board->countWhite() >= 44))
        finalMode = true;

    for (int i = 0; i < 64; i++)
        history_table[i / 8][i % 8] = 0;

    Move bestMove(-1, -1);
    for (int i = 1; i < 20; i++) {
        for (int j = 0; j < 64; j++)
            history_table[j / 8][j % 8] /= 2;

        //yeayeah cerr << "PLY IS " << i << ": ";
        try {
            Move move(-1, -1);
            int minim = negamax(board, ourSide, i, -(INT_MAX - 1), INT_MAX - 1, elapsed_moves, move);
            //cerr << "Minimum score is " << minim << " with the move " << (int) move.x << ", " << (int) move.y << "\n";
            bestMove = move;
        } catch(...) {
            //cerr << "Quit early!\n\n";
            break;
        }
    }

    return bestMove;
}

/*
 * Calculates highest-scoring move using a negamax algorithm to arbitrary depth.
 */
 ////// MODIFIED FOR NEGASCOUT //////
int Player::negamax(Board *current, Side player, int depth, int a, int b,
                    int elapsedMoves, Move &ret /*pseudo-return-value.*/)
{
    //it++;
    int old_alpha = a;

    if (depth == 8 && outOfTime())
        throw 1;

    if (depth == 0 || current->isDone()) {
        return current->score(player, elapsedMoves);
    }

    Bucket* bucket = nullptr;

    //bucket = try_retrieve(current, player);
    bucket = nullptr;

    if (bucket != nullptr && bucket->depth >= depth) {
        if (bucket->exactness == EXACT) {
            ret = bucket->best_move;
            //si++;
            return bucket->value;
        } else if (bucket->exactness == LOWER) {
            if (bucket->value > a)
                a = bucket->value;
        } else if (bucket->exactness == UPPER) {
            if (bucket->value < b)
                b = bucket->value;
        }

        if (a >= b) {
            //si++;
            ret = bucket->best_move;
            return bucket->value;
        }
    }

    Move dummy(-1, -1);

    if (!current->hasMoves(player))
        return -negamax(current, OPPOSITE(player), depth - 1, -b, -a,
                elapsedMoves + 1, dummy);

    if (bucket != nullptr) {
        Move& move = bucket->best_move;
        Board *copy = current->copyDoMove(&move, player);
        int score = -negamax(copy, OPPOSITE(player), depth - 1, -b, -a,
                             elapsedMoves + 1, dummy);

        delete copy;

        if (score >= a) {
            ret = move;
            a = score;
        }

        if (a >= b) { // no longer worth pursuing branch
            //si++;

            if (ret.x != -1 && ret.y != -1)
                history_table[ret.x][ret.y] += pow(2, depth);

            return a;
        }
    }

    vector<Move> moves = current->getMoves(player);
    sort(moves.begin(), moves.end(), cmp);

    if (moves.size() > 0) {
        Move& move = moves[0];
        Board *copy = current->copyDoMove(&move, player);
        int score = -negamax(copy, OPPOSITE(player), depth - 1, -b, -a,
                             elapsedMoves + 1, dummy);

        delete copy;

        if (score >= a) {
            ret = move;
            a = score;
        }

        if (a >= b) { // no longer worth pursuing branch
            try_save(current, a, ret, old_alpha, b, depth, player);

            if (ret.x != -1 && ret.y != -1)
                history_table[ret.x][ret.y] += pow(2, depth);

            return a;
        }
    }

    for (auto iter = moves.begin() + 1; iter != moves.end(); iter++) {
        Move& move = *iter;
        Board *copy = current->copyDoMove(&move, player);
        int score = -negamax(copy, OPPOSITE(player), depth - 1, -a-1, -a,
                             elapsedMoves + 1, dummy);

        if (a < score && score < b) {
            //ft++;
            score = -negamax(copy, OPPOSITE(player), depth - 1, -b, -score,
                             elapsedMoves + 1, dummy);
        }
        delete copy;

        if (score > a) {
            ret = move;
            a = score;
        }

        if (a >= b) // no longer worth pursuing branch
            break;
    }

    if (ret.x != -1 && ret.y != -1)
        history_table[ret.x][ret.y] += pow(2, depth);

    try_save(current, a, ret, old_alpha, b, depth, player);
    return a;
}
