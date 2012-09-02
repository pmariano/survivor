#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "font.h"
#include "app.h"
#include "sound.h"
#include "render.h"
#include "movement.h"
#include "aStarLibrary.h"

#define FPS 40
#define MAX(a,b) ((a)>(b)?(a):(b))

void finishHim(App *app){
	app->state = STATE_EXIT;
}

void gameInit(App *app){
  app->game.start = SDL_GetTicks();
  app->game.spawnTime = app->game.start;
	/**
	 * Player 1 init settings
	 * */
	Body *p1body = &app->game.player1.body;
	p1body->ang_vel = 0.3;
	p1body->max_vel = 5;
	p1body->angle = 0;
	player_spawn_pos(&app->game, &p1body->pos.x, &p1body->pos.y);

	/**
	 * Player 2 init settings
	 * */
	Body *p2body = &app->game.player2.body;

	p2body->ang_vel = 0.3;
	p2body->max_vel = 5;
	p2body->angle = 1;
  app->game.latest_enemy_updated = 0;
  printf("caires  %d\n\n\n", app->game.latest_enemy_updated);
  
	player_spawn_pos(&app->game, &p2body->pos.x, &p2body->pos.y);
  int i = 0;
  for(;i < ENEMY_COUNT; i++)
  {
    app->game.enemies[i].state = ENEMY_DEAD;
  }
}

void resetApp(App *app){
	gameInit(app);
	//reset here other stuffs
}

void pauseOrJoinTheGame(App *app, Player *player){
	if(player->state == PLAYER_READY){
		app->state = STATE_PAUSED;
		app->menu.selected = MENU_RESUME;
	} else{
		player->state = PLAYER_READY;
	}
}

void bindGameplayKeysDown(App *app, SDLKey *key){
	Player *player1 = &app->game.player1;
	Player *player2 = &app->game.player2;

	switch(*key){
		case SDLK_1:
			pauseOrJoinTheGame(app, player1);
			break;
		case SDLK_2:
			pauseOrJoinTheGame(app, player2);
			break;
		case SDLK_0:
			app->debug ^= 1;
			break;
		case SDLK_ESCAPE:
			app->state = STATE_PAUSED;
			break;
	}
}

void bindMenuKeysDown(App *app, SDLKey *key){
	Player *player1 = &app->game.player1;
	Player *player2 = &app->game.player2;
	Menu *menu = &app->menu;
	int firstMenu = MENU_NEW_GAME;

	/**
	 * when game is paused there is more menu options
	 * */
	if(app->state == STATE_PAUSED){
		firstMenu = MENU_RESUME;
	}

	switch(*key){
		case SDLK_1:
			player1->state = PLAYER_READY;
			break;
		case SDLK_2:
			player2->state = PLAYER_READY;
			break;
		case SDLK_UP:
		case SDLK_q:
		case SDLK_t:
			if(menu->selected != firstMenu){
				menu->selected--;
			}
			break;
		case SDLK_DOWN:
		case SDLK_w:
		case SDLK_y:
			if(menu->selected < MENU_COUNT - 1){
				menu->selected++;
			}
			break;
		case SDLK_a:
		case SDLK_z:
		case SDLK_RETURN:
			if(app->state == STATE_CREDITS){
				if(app->credits == CREDITS_SOUND){
					app->state = app->stateBeforeCredits;
					app->credits = CREDITS_TEAM;
				} else{
					app->credits = CREDITS_SOUND;
				}

				break;
			}

			if(*key == SDLK_z){
				player2->state = PLAYER_READY;
			} else {
				player1->state = PLAYER_READY;
			}

			if(menu->selected == MENU_NEW_GAME){
				resetApp(app);
				app->state = STATE_PLAYING;
			} else if (menu->selected == MENU_QUIT){
				finishHim(app);
			} else if(menu->selected == MENU_CREDITS){
				AppState s = app->state;
				app->stateBeforeCredits = s;
				app->state = STATE_CREDITS;
			} else if(menu->selected == MENU_RESUME){
				app->state = STATE_PLAYING;
			}
			break;
		case SDLK_ESCAPE:
			app->state = STATE_PAUSED;
			break;
	}
}

/**
 * Keystate is responsible to handle pressed keys
 */
void bindGameplayKeystate(App *app){
	Player *player1 = &app->game.player1;
	Player *player2 = &app->game.player2;

	Uint8 *keystate;
	keystate = SDL_GetKeyState(NULL);

	/**
	 * Player 1 settings:
	 * Q = UP; W = DOWN; E = LEFT; R = RIGHT
	 * A = ATTACK
	 * S = SECONDARY ATTACK
	 * */

	player_move(&app->game, &player1->body,
		keystate[SDLK_UP] || keystate[SDLK_q],
		keystate[SDLK_RIGHT] || keystate[SDLK_r],
		keystate[SDLK_DOWN] || keystate[SDLK_w],
		keystate[SDLK_LEFT] || keystate[SDLK_e]
	);

	/**
	 * Player 2 settings:
	 * T = UP; Y = DOWN; U = LEFT;I = RIGHT
	 * Z = ATTACK
	 * S = SECONDARY ATTACK
	 * */
	player_move(&app->game, &player2->body,
		keystate[SDLK_KP6] || keystate[SDLK_i],
		keystate[SDLK_KP8] || keystate[SDLK_t],
		keystate[SDLK_KP4] || keystate[SDLK_u],
		keystate[SDLK_KP5] || keystate[SDLK_KP2] || keystate[SDLK_y]
	);
}

void bindKeyboard(App *app)
{
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type) {
			case SDL_KEYDOWN:
				if (app->state == STATE_PLAYING){
					bindGameplayKeysDown(app, &event.key.keysym.sym);
				} else{
					bindMenuKeysDown(app, &event.key.keysym.sym);
				}
		}
	}

	if (app->state == STATE_PLAYING){
		bindGameplayKeystate(app);
	}
}

void handleDelay(Uint32 start) {
	Uint32 end = SDL_GetTicks();
	int actual_delta = end - start;
	int expected_delta = 1000/FPS;
	int delay = MAX(0, expected_delta - actual_delta);
	SDL_Delay(delay);
}

void spawnEnemy(App *app)
{
  Game *game = &app->game;
  Enemy *enemy = NULL;
  int x,y;
  
  int i = 0;
  for(; i < ENEMY_COUNT; i++)
  {
    if(game->enemies[i].state == ENEMY_DEAD)
    {
      enemy = &game->enemies[i];
      enemy->state = ENEMY_LIVE;
      break;
    }
  }
  
  if(enemy != NULL && enemy_spawn_pos(game, &x,&y))
  {
    enemy->image = game->enemy_class_medic.image;
    Body *enemybody = &enemy->body;
    enemybody->ang_vel = 0.05;
    enemybody->max_vel = 2.5;
    enemybody->angle = 1;
    enemybody->pos.x = x;
    enemybody->pos.y = y;
  }
}

void loadMap(App *app, int map_index) {
	char image_path[256];
	char hit_path[256];
	sprintf(image_path, "data/map%d.bmp", map_index);
	sprintf(hit_path, "data/map%d_hit.bmp", map_index);
	app->game.board.image = IMG_Load(image_path);
	app->game.board.hit = IMG_Load(hit_path);
	moveInit(app);
}

int trace_bullet_shot(App *app, Body *body, int x, int y)
{
	Uint8 *p = ((Uint8*)app->screen->pixels) + (x*app->screen->format->BytesPerPixel+y*app->screen->pitch);
	p[0] = 0xff;
	p[1] = 0xff;
	int hit = app->game.board.crowd[x/tileSize][y/tileSize];
	if(hit) {
		if(hit>4) {
			int i = hit - 4;
			hit(body, &app->game.enemies[i].body);

			// move to hit()
			SDL_Rect rect = {
				x - body->item.hit_image.w/2,
				y - body->item.hit_image.h/2,
				body->item.hit_image.w,
				body->item.hit_image.h
			};
			SDL_BlitSurface(body->item.hit_image, NULL, app->screen, &rect);
		}
	}
	return hit;
}


int trace(App *app, Body *body, int range, int (*draw)(App *app, int x, int y))
{
	int x1,int y1,int x2,int y2;
	int dx, dy, i, e;
	int incx, incy, inc1, inc2;
	int x,y;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if(dx > dy)
	{
		if(draw(app,body,x,y)) return 1;
		e = 2*dy - dx;
		inc1 = 2*( dy -dx);
		inc2 = 2*dy;
		for(i = 0; i < dx; i++)
		{
			if(e >= 0)
			{
				y += incy;
				e += inc1;
			}
			else e += inc2;
			x += incx;
			if(draw(app,body,x,y)) return 1;
		}
	}
	else
	{
		if(draw(app,body,x,y)) return 1;
		e = 2*dx - dy;
		inc1 = 2*( dx - dy);
		inc2 = 2*dx;
		for(i = 0; i < dy; i++)
		{
			if(e >= 0)
			{
				x += incx;
				e += inc1;
			}
			else e += inc2;
			y += incy;
			if(draw(app,body,x,y)) return 1;
		}
	}
	return 0;
}


int main(int argc, char* args[] )
{
	srand(time(NULL));
	App app;
	app.debug = 0;
	memset(&app, 0, sizeof(app));

	app.state = STATE_MENU;
	app.menu.selected = MENU_NEW_GAME;
	app.credits = CREDITS_TEAM;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;
	init_font();
  InitializePathfinder();
	renderInit(&app);
	loadMap(&app, 0); // calls moveInit
	soundInit();

	while(app.state != STATE_EXIT){
	  Uint32 startTime = SDL_GetTicks();
		movePrepare(&app);
		bindKeyboard(&app);

		if (app.state == STATE_PLAYING){
			playRandomMusic();
      Uint32 elapsed = startTime - app.game.spawnTime;
      if(elapsed > 1000)
      {
        spawnEnemy(&app);
        app.game.spawnTime = startTime;
      }
      move_enemies(&app);
			render(&app);
		} else if (app.state == STATE_CREDITS) {
			renderCredits(&app);
		}	else {
			renderMenu(&app);
			playMusic("menu.mp3", -1);
		}
		handleDelay(startTime);
	}

	return 0;
}


