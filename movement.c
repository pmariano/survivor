#include "movement.h"
#include "aStarLibrary.h"

#define ATAN2(dx,dy) ((int)(360+atan2(-(dy),(dx))*180/M_PI)%360)

void moveInit(App *app)
{
	int x,y;

	memset(walkability, 0, sizeof(walkability));
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			if((rand() % 10) == 0) walkability[x][y] = 1;
		}
	}
}

void angle_rotate(float *a0_base, float a1, float f)
{
	float a0 = *a0_base;
	if(fabs(a1 - a0) > 180) {
		if(a0 < a1)
			a0 += 360;
		else
			a1 += 360;
	}
	a0 = (360+a0)*(1-f) + f*(360+a1);
	*a0_base = a0 > 360 ? a0 - 360 : a0;
}

void body_move(Game *game, Body *body, float angle)
{
	if(body->action == ACTION_DEATH){
		return;
	}

	float v = body->max_vel;

	angle_rotate(&body->angle, angle, body->ang_vel);
	float a = body->angle * M_PI / 180.;
	float dx = (int)(cos(a) * v * 100)/100.;
	float dy = (int)(sin(a) * v * 100)/100.;
	body->pos.x += dx;
	body->pos.y -= dy;
	printf("body: angle: %f a: %f 2pi:%f y:%f\n", body->angle, a, 2*M_PI, sin(a) * v);
	//body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;
}

void move_enemies(App *app)
{
  int i = 0;
  for(; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].state == ENEMY_LIVE)
    {
        Body *enemy_body = &app->game.enemies[i].body;
        pathStatus[i] = FindPath(i, 
            enemy_body->pos.x, 
            enemy_body->pos.y, 
            app->game.player1.body.pos.x, 
            app->game.player1.body.pos.y);
        if(pathStatus[i] == found)
        {
          ReadPath(i, enemy_body->pos.x, enemy_body->pos.y, 1);    
          int dx = xPath[i] - enemy_body->pos.x;
          int dy = yPath[i] - enemy_body->pos.y;
          float angle = ATAN2(dx,dy);
          body_move(&app->game, enemy_body, angle);
        }
    }
  }
}

void player_move(Game *game, Body *body, int up, int right, int down, int left)
{
    int dx=right-left;
    int dy=down-up;
    if(dx||dy) {
        float angle = ATAN2(dx,dy);
        body_move(game, body, angle);

    }
}

