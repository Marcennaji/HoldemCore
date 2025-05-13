#ifndef PLAYER_POSITION_H
#define PLAYER_POSITION_H

enum PlayerPosition {
    UNKNOWN = 0,
    SB = 1,
    BB = 2,
    UTG = 3,
    UTG_PLUS_ONE = 4,
    UTG_PLUS_TWO = 5,
    MIDDLE = 6,
    MIDDLE_PLUS_ONE = 7,
    LATE = 8,
    CUTOFF = 9,
    BUTTON = 10
};

#endif // PLAYER_POSITION_H