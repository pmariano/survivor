#include "movement.h"

void angle_rotate(int *a0_base, int a1, float f)
{
	int a0 = *a0_base;
	if(fabs(a1 - a0) > 180) {
		if(a0 < a1)
			a0 += 360;
		else
			a1 += 360;
	}
	*a0_base = (int)((720+a0)*(1-f) + f*(720+a1)) % 360;
}

void body_move(Game *game, Body *body, int angle)
{
	if(body->action == ACTION_DEATH){
		return;
	}
	printf("body: angle: %i\n", body->angle);

	float v = body->max_vel;

	angle_rotate(&body->angle, angle, body->ang_vel);
	float a = body->angle * M_PI / 180;
	body->pos.x += cos(a) * v;
	body->pos.y -= sin(a) * v;
	//body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;
}

#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360)
void player_move(Game *game, Body *body, int up, int right, int down, int left)
{
    float dx=right-left;
    float dy=down-up;
    if(fabs(dx)>0.1||fabs(dy)>0.1) {
        int angle = ATAN2(dx,dy);
        body_move(game, body, angle);

    }
}
