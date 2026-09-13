#ifndef CONSTS_H
#define CONSTS_H

int GAME_WIDTH = 300;
int GAME_HEIGHT = 900;

typedef enum {
    NONE,
    ROCKET_LAUNCH,
    AVALANCHE,
    PATHBREAKER
} Game;
Game CURRENT_GAME = NONE;
#endif




