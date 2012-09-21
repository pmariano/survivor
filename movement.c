#include "movement.h"
#include "aStarLibrary.h"

#define ATAN2(dx,dy) ((int)(720+atan2(-(dy),(dx))*180/M_PI)%360) // FIXME wrap angle properly

void movePrepare(App *app)
{
	int i;
	memcpy(walkability, app->game.board.wall, sizeof(app->game.board.wall)); // 1 = totaly unwalkable
	memcpy(app->game.board.crowd, app->game.board.wall, sizeof(app->game.board.wall));
	memcpy(app->game.board.hittable, app->game.board.air, sizeof(app->game.board.air));

	for(i=0; i < app->game.board.spawn_count; i++)
	{
		if(app->game.board.spawn[i].open)
		{
			int x = app->game.board.spawn[i].x;
			int y = app->game.board.spawn[i].y;
			walkability[x][y] = 0; // totally walkable
		}
	}

	if( app->game.player1.body.status == BODY_ALIVE) {
		int x = app->game.player1.body.pos.x/tileSize;
		int y = app->game.player1.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 2;
	}

	if( app->game.player2.body.status == BODY_ALIVE) {
		int x = app->game.player2.body.pos.x/tileSize;
		int y = app->game.player2.body.pos.y/tileSize;
		app->game.board.crowd[x][y] = 3;
	}

	for(i=0; i < ENEMY_COUNT; i++)
	{
		if(app->game.enemies[i].body.status == BODY_ALIVE)
		{
			int x = app->game.enemies[i].body.pos.x/tileSize;
			int y = app->game.enemies[i].body.pos.y/tileSize;
			app->game.board.crowd[x][y] = 4+i;
			app->game.board.hittable[x][y] = 4+i;
			walkability[x][y] = mapWidth; // 1% walkable
		}
	}
				
}


void moveInit(App *app)
{
	int x,y;
	SDL_Surface *hit = app->game.board.hit;
	memset(app->game.board.wall, 0, sizeof(app->game.board.wall));
	memset(app->game.board.air, 0, sizeof(app->game.board.air));
	memset(app->game.board.powerup, 0, sizeof(app->game.board.powerup));
	memset(app->game.board.safearea, 0, sizeof(app->game.board.safearea));
	memset(app->game.board.spawn_map, 0, sizeof(app->game.board.spawn_map));
	app->game.board.spawn_count = 0;
	for (x=0; x < mapWidth;x++) {
		for (y=0; y < mapHeight;y++) {
			Uint32 *p = (Uint32*)( ((Uint8*)hit->pixels) + (x*hit->format->BytesPerPixel+y*hit->pitch) );
			Uint8 r,g,b;
			SDL_GetRGB(*p, hit->format, &r, &g, &b);
			int threshold = 0x40;
			int walk = b+g>r+threshold;
			int fly = r+g+b>threshold;
			int safe = g>r+b+threshold;
			app->game.board.wall[x][y] = !walk;
			app->game.board.air[x][y] = !fly;
			app->game.board.safearea[x][y] = safe;
			if(walk && (x==0 || y==0 || x==mapWidth-1 || y==mapHeight-1
				|| x == (app->game.board.wave[app->game.board.wave_index].w)-1
				|| y == (app->game.board.wave[app->game.board.wave_index].h)-1
			)) {
				app->game.board.spawn_map[x][y]=1;
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

inline int is_air(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;
	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	return game->board.hittable[x][y];
}
inline int is_solid(Game *game, Body *body, int x, int y)
{
	x/=tileSize;
	y/=tileSize;

	if(x<0 || y<0 || x>=mapWidth || y>=mapHeight)
		return 1;
	if(body->pos.x/tileSize == x && body->pos.y/tileSize == y)
		return 0;
	return game->board.crowd[x][y];
}
inline int is_empty(Game *game, Body *body, int x, int y)
{
	return !is_solid(game, body,x,y);
}

void body_move(Game *game, Body *body, float angle, float vel)
{
	if(body->status== BODY_DEAD){
		return;
	}

	float v = body->max_vel * vel;

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
    if(app->game.enemies[i].body.status == BODY_ALIVE
	&& app->game.enemies[i].body.life <= 0) {
		app->game.enemies[i].body.status = BODY_DEAD;
	}
  }

  int n=30; // ENEMY_COUNT
  for(i=0; i < n; i++) 
  {
    int id = app->game.latest_enemy_updated = ( app->game.latest_enemy_updated + 1 ) % ENEMY_COUNT;
    int crazy = id*2;
    if(app->game.enemies[id].body.status == BODY_ALIVE)
    {
        Body *enemy_body = &app->game.enemies[id].body;
		n++;
		pathLength[crazy] = 9999;
		pathLength[crazy+1] = 9999;

		if(app->game.player1.body.status == BODY_ALIVE) {
			//printf("find %d=%d\n", id, crazy);
			pathStatus[crazy] = FindPath(crazy,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.player1.body.pos.x,
				app->game.player1.body.pos.y);
		}

		if(app->game.player2.body.status == BODY_ALIVE) {
			//printf("find %d=%d\n", id, crazy+1);
			pathStatus[crazy+1] = FindPath(crazy+1,
				enemy_body->pos.x,
				enemy_body->pos.y,
				app->game.player2.body.pos.x,
				app->game.player2.body.pos.y);
		}


        if(pathLength[crazy] < pathLength[crazy+1]){
          app->game.enemies[id].pathfinder = crazy;
          app->game.enemies[id].pathfinder_other = crazy+1;
          app->game.enemies[id].target = &app->game.player1.body;
        }else{
          app->game.enemies[id].pathfinder = crazy+1;
          app->game.enemies[id].pathfinder_other = crazy;
          app->game.enemies[id].target = &app->game.player2.body;
        }


#if 0
		{
			extern int* pathBank [numberPeople+1];
			int k;
			int j = app->game.enemies[id].pathfinder;
			printf("%d (%d)", j, pathLength[j]);
			for(k=0; k<pathLength[j]; k+=2) {
				printf("%d,%d; ", pathBank[j][k], pathBank[j][k+1]);
			}
			printf("\n");
		}
#endif 

    }
  }

  for(i = 0; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].body.status == BODY_ALIVE)
    {
        Body *enemy_body = &app->game.enemies[i].body;
		int crazy = app->game.enemies[i].pathfinder;
        if(pathStatus[crazy] == found)
        {
			//printf("read %d=%d\n", i, crazy);
          int reach = ReadPath(crazy, enemy_body->pos.x, enemy_body->pos.y, tileSize*1.25);
          int dx = xPath[crazy] - enemy_body->pos.x;
          int dy = yPath[crazy] - enemy_body->pos.y;
          float angle = ATAN2(dx,dy);
          body_move(&app->game, enemy_body, angle, .25+.75*rand()/(float)RAND_MAX);

		  if(reach){
				//printf("reach %d=%d %d,%d\n", i, crazy, dx, dy);
				pathStatus[app->game.enemies[i].pathfinder_other] = notStarted;
				hit(app, enemy_body, app->game.enemies[i].target);
		  }
		}
	}
  }
}



void player_move(App *app, Body *body, int up, int right, int down, int left, int halt)
{
    Game game = app->game; 
    aim(app, body);
    int dx=right-left;
    int dy=down-up;
    if(dx||dy) {
        float angle = ATAN2(dx,dy);
        body_move(&game, body, angle, !halt);
    }
}


int enemy_spawn_pos(Game *game, int *x, int *y) {
	if(!game->board.spawn_count) return 0;
	int i = rand() % game->board.spawn_count;
	// printf("%d %d\n", i, rand() % game->board.spawn_count);

	*x = game->board.spawn[i].x*tileSize+tileSize*3/4;
	*y = game->board.spawn[i].y*tileSize+tileSize*3/4;
	return 1;
}

int player_spawn_pos(Game *game, Uint16 *x, Uint16 *y)
{
	int i;
	for(i=0; i< 100; i++) {
		int x1 = rand() % mapWidth;
		int y1 = rand() % mapHeight;
		if(game->board.safearea[x1][y1] && !game->board.crowd[x1][y1]) {
			*x = x1 * tileSize + tileSize/2;
			*y = y1 * tileSize + tileSize/2;
			return 1;
		}
	}
	return 0;
}

int powerup_spawn_pos(Game *game, int *x, int *y) {
	int i;
	for(i=0; i< 50; i++) {
		int x1 = rand() % mapWidth;
		int y1 = rand() % mapHeight;
		if(game->board.safearea[x1][y1] && !game->board.powerup[x1][y1]) {
			*x = x1 * tileSize + tileSize/2;
			*y = y1 * tileSize + tileSize/2;
			return 1;
		}
	}
	return 0;
}

