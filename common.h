#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <cstdint>
#include <climits>
#include <iostream>

enum Side {
    WHITE, BLACK
};

#define OPPOSITE(x) ((x) == BLACK ? WHITE : BLACK)

class Move {
public:
    int8_t x, y;
    Move(int x, int y) : x(x), y(y) {};
    Move(const Move&) = default;
    ~Move() = default;

    inline int getX() { return x; }
    inline int getY() { return y; }

    inline void setX(int x) { this->x = x; }
    inline void setY(int y) { this->y = y; }
};

#endif
