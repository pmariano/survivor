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
	/**
	 * Player 1 init settings
	 * */
	Body *p1body = &app->game.player1.body;
	p1body->ang_vel = 0.3;
	p1body->max_vel = 5;
	p1body->angle = 0;
	p1body->pos.x = 1204/2+15;
	p1body->pos.y = 768/2+15;

	/**
	 * Player 2 init settings
	 * */
	Body *p2body = &app->game.player2.body;

	p2body->ang_vel = 0.3;
	p2body->max_vel = 5;
	p2body->angle = 1;
	p2body->pos.x = 1204/2+40;
	p2body->pos.y = 768/2+40;

	/**
	 * Enemy Body
	 *
	 * */
  Body *enemybody = &app->game.enemy.body;
	enemybody->ang_vel = 0.05;
	enemybody->max_vel = 2.5;
	enemybody->angle = 1;
	enemybody->pos.x = 1204/2-550;
	enemybody->pos.y = 768/2+80;
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
			finishHim(app);
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
				app->state = app->stateBeforeCredits;
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
			finishHim(app);
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

int main(int argc, char* args[] )
{
	App app;
	app.debug = 0;
	memset(&app, 0, sizeof(app));

	app.state = STATE_MENU;
	app.menu.selected = MENU_NEW_GAME;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;
	init_font();
  InitializePathfinder();
	renderInit(&app);
	gameInit(&app);
	moveInit(&app);
	soundInit();


	while(app.state != STATE_EXIT){
	  Uint32 startTime = SDL_GetTicks();
		bindKeyboard(&app);

		if (app.state == STATE_PLAYING){
			pathStatus[1] = FindPath(1,
				app.game.enemy.body.pos.x,
				app.game.enemy.body.pos.y,
				app.game.player1.body.pos.x,
				app.game.player1.body.pos.y);
			enemy_move(&app.game, &app.game.enemy.body);
			render(&app);
		} else if (app.state == STATE_CREDITS) {
			renderCredits(&app);
		}	else {
			renderMenu(&app);
			playMusic("sounds/menu1.mp3");
		}
		handleDelay(startTime);
	}

	return 0;
}


