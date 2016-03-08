#ifndef __COMMON_H__
#define __COMMON_H__

#include <vector>
#include <cstdint>

enum Side {
    WHITE, BLACK
};

class Move {
    uint8_t x, y;
public:
    Move(int x, int y) {
        this->x = x;
        this->y = y;
    }
    ~Move() {}

    inline int getX() { return x; }
    inline int getY() { return y; }

    inline void setX(int x) { this->x = x; }
    inline void setY(int y) { this->y = y; }
};

#endif
