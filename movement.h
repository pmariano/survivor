#include <math.h>
#include "app.h"

void angle_rotate(float *a0_base, float a1, float f);
void body_move(Game *game, Body *body, float angle);
void player_move(Game *game, Body *body, int up, int right, int down, int left);
void moveInit(App *app);
void movePrepare(App *app);
int player_spawn_pos(Uint16 *x, Uint16 *y);
int enemy_spawn_pos(Game *game, int *x, int *y);
int powerup_spawn_pos(Game *game, int *x, int *y);
