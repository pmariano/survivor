#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>

#include "font.h"
#include "app.h"
#include "render.h"
#include "movement.h"



void finishHim(App *app){
	app->state = STATE_EXIT;
}

void bindGameplayKeysDown(App *app, SDLKey *key){
	Player *player1 = &app->game.player1;
	Player *player2 = &app->game.player2;

	switch(*key){
		case SDLK_1:
			player1->state = PLAYER_READY;
			break;
		case SDLK_2:
			player2->state = PLAYER_READY;
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
			if(menu->selected != 0){
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
			if(*key == SDLK_z){
				player2->state = PLAYER_READY;
			} else {
				player1->state = PLAYER_READY;
			}
			if(menu->selected == MENU_NEW_GAME){
				app->state = STATE_PLAYING;
			} else if (menu->selected == MENU_QUIT){
				finishHim(app);
			} else if(menu->selected == MENU_CREDITS){
				app->state = STATE_CREDITS;
				renderCredits(app);
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
		keystate[SDLK_LEFT] || keystate[SDLK_e],
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
				if (app->state == STATE_MENU){
					bindMenuKeysDown(app, &event.key.keysym.sym);
				} else{
					bindGameplayKeysDown(app, &event.key.keysym.sym);
				}
		}
	}

	if (app->state != STATE_MENU){
		bindGameplayKeystate(app);
	}
}

int hasNoReadyPlayers(Game *game) {
	return !game->player1.state == PLAYER_READY && !game->player2.state == PLAYER_READY;
}

void gameInit(App *app){
	app->game.player1.state = PLAYER_IDLE;

	/**
	 * Player 1 init settings
	 * */
	Body *p1body = &app->game.player1.body;
	p1body->ang_vel = 0.3;
	p1body->max_vel = 5;
	p1body->angle = 1;
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
}

int main(int argc, char* args[] )
{
	App app;
	Menu menu;
	app.state = STATE_MENU;
	app.menu.selected = 0;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;
	init_font();
	renderInit(&app);
	gameInit(&app);
	while(app.state != STATE_EXIT){
		bindKeyboard(&app);

		if (app.state == STATE_MENU){
			if(hasNoReadyPlayers(&app.game)){
				printf("No players at moment \n");
			}
			renderMenu(&app);
		} else {
			render(&app);
		}
	}

	return 0;
}


