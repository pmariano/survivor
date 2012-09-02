#include <math.h>
#include "app.h"

void angle_rotate(float *a0_base, float a1, float f);
void body_move(Game *game, Body *body, float angle);
void player_move(Game *game, Body *body, int up, int right, int down, int left);
