#include "movement.h"
#include "aStarLibrary.h"

#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360) // FIXME wrap angle properly

void movePrepare(App *app)
{
	int i;
	memcpy(app->game.board.crowd, app->game.board.wall, sizeof(app->game.board.wall));
#if 1
	if( app->game.player1.state == PLAYER_READY) {
		int x = app->game.player1.body.pos.x/tileSize;
		int y = app->game.player1.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 2;
	}
	if( app->game.player2.state == PLAYER_READY) {
		int x = app->game.player2.body.pos.x/tileSize;
		int y = app->game.player2.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 3;
	}
	for(i=0; i < ENEMY_COUNT; i++)
	{
		if(app->game.enemies[i].state == ENEMY_LIVE)
		{
			int x = app->game.enemies[i].body.pos.x/tileSize;
			int y = app->game.enemies[i].body.pos.y/tileSize;
			app->game.board.crowd[x][y] = 4+i;
		}

	}
#endif
}


void moveInit(App *app)
{
	int x,y;
	SDL_Surface *hit = app->game.board.hit;
	memset(app->game.board.wall, 0, sizeof(app->game.board.wall));
	memset(app->game.board.powerup, 0, sizeof(app->game.board.powerup));
	app->game.board.spawn_count = 0;
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			Uint8 *p = ((Uint8*)app->game.board.hit->pixels) + (x*hit->format->BytesPerPixel+y*hit->pitch);
			// printf("%d,%d: %d %d %d %d\n", x,y, hit->format->BytesPerPixel,p[0], p[1], p[2]);
			app->game.board.wall[x][y] = !p[0];
			walkability[x][y] = !(p[0]||p[2]);
			if(p[2]) {
				app->game.board.spawn[app->game.board.spawn_count].x = x;
				app->game.board.spawn[app->game.board.spawn_count].y = y;
				app->game.board.spawn[app->game.board.spawn_count].open = 1;
				app->game.board.spawn_count++;
			}
		}
	}
	movePrepare(app);
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
	// FIXME wrap angle properly
	a0 = (720+a0)*(1-f) + f*(720+a1);
	while(a0>360) a0-=360;
	*a0_base = a0;
}

int is_solid(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;

	if(body->pos.x/tileSize == x && body->pos.y/tileSize == y)
		return 0;
	return game->board.crowd[x][y];
}
int is_empty(Game *game, Body *body, int x, int y)
{
	return !is_solid(game, body,x,y);
}

void body_move(Game *game, Body *body, float angle)
{
	if(body->status== BODY_DEAD){
		return;
	}

	float v = body->max_vel;

	angle_rotate(&body->angle, angle, body->ang_vel);
	float a = body->angle * M_PI / 180.;
	float dx = (int)(cos(a) * v * 100)/100.;
	float dy = (int)(sin(a) * v * 100)/100.;
	int x0 = body->pos.x;
	int y0 = body->pos.y;
	int x1 = x0 + dx;
	int y1 = y0 - dy;

	if(is_empty(game, body,x1,y1)) {
		body->pos.x = x1;
		body->pos.y = y1;
	} else {
		if(is_empty(game, body,x1,y0)) {
			body->pos.x = x1;
		} else if(is_empty(game, body,x0,y1)) {
			body->pos.y = y1;
		}
	}

	//printf("body: angle: %f a: %f 2pi:%f y:%f\n", body->angle, a, 2*M_PI, sin(a) * v);
	//body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;
}

void move_enemies(App *app)
{
  int i;
  for(i=0; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].state == ENEMY_LIVE
	&& app->game.enemies[i].body.life <= 0) {
		app->game.enemies[i].state = ENEMY_DEAD;
	}
  }

  for(i=0; i < ENEMY_COUNT; i++)
  {
    int id = app->game.latest_enemy_updated = ( app->game.latest_enemy_updated + 1 ) % ENEMY_COUNT;
    if(app->game.enemies[id].state == ENEMY_LIVE)
    {
        Body *enemy_body = &app->game.enemies[id].body;
        pathStatus[id] = FindPath(id,
            enemy_body->pos.x,
            enemy_body->pos.y,
            app->game.player1.body.pos.x,
            app->game.player1.body.pos.y);
        break;
    }
  }

  for(i = 0; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].state == ENEMY_LIVE)
    {
        Body *enemy_body = &app->game.enemies[i].body;
        if(app->game.latest_enemy_updated+1 == i)
        {
          pathStatus[i] = FindPath(i,
              enemy_body->pos.x,
              enemy_body->pos.y,
              app->game.player1.body.pos.x,
              app->game.player1.body.pos.y);
          app->game.latest_enemy_updated = i;
        }
        if(pathStatus[i] == found)
        {
          int reach = ReadPath(i, enemy_body->pos.x, enemy_body->pos.y, 1);
          int dx = xPath[i] - enemy_body->pos.x;
          int dy = yPath[i] - enemy_body->pos.y;
          float angle = ATAN2(dx,dy);
          body_move(&app->game, enemy_body, angle);

		  if(reach){
			  hit(app, enemy_body, &app->game.player1.body);
		  }
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


int enemy_spawn_pos(Game *game, int *x, int *y) {
	if(!game->board.spawn_count) return 0;
	int i = rand() % game->board.spawn_count;
	// printf("%d %d\n", i, rand() % game->board.spawn_count);

	*x = game->board.spawn[i].x*tileSize+tileSize/2;
	*y = game->board.spawn[i].y*tileSize+tileSize/2;
	return 1;
}

int player_spawn_pos(Game *game, Uint16 *x, Uint16 *y)
{
	int i;
	for(i=0; i< 100; i++) {
		int x1 = rand() % mapWidth;
		int y1 = rand() % mapHeight;
		if(!game->board.crowd[x1][y1]) {
			*x = x1 * tileSize + tileSize/2;
			*y = y1 * tileSize + tileSize/2;
			return 1;
		}
	}
	return 0;
}

// FIXME powerups aparecendo em cima da mese
// FIXME powerups aparecendo uns sobre os outros
int powerup_spawn_pos(Game *game, int *x, int *y) {
	int i;
	for(i=0; i< 10; i++) {
		int x1 = rand() % mapWidth;
		int y1 = rand() % mapHeight;
		if(!game->board.wall[x1][y1] && !game->board.powerup[x1][y1]) {
			*x = x1 * tileSize + (rand() % tileSize);
			*y = y1 * tileSize + (rand() % tileSize);
			return 1;
		}
	}
	return 0;
}

