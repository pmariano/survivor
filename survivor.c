#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "font.h"
#include "app.h"

void move(char* msg){
  printf("%s\n",msg);
}

void finishHim(App *app){
  printf("FINISH HIM");
  app->state = STATE_EXIT;
}

void handleArcadeKeys(App *app, SDLKey *key){
  Player player1 = app->game.player1;
  Player player2 = app->game.player2;
  switch(*key){
	/**
	 * Player 1 settings:
	 * Q = UP; W = DOWN; E = LEFT; R = RIGHT
	 * A = ATTACK
	 * S = SECONDARY ATTACK
	 * */
	case SDLK_1:
	  player1.state = PLAYER_READY;
	  break;
	case SDLK_q:
	  move("up");
	  break;
	case SDLK_w:
	  move("down");
	  break;
	case SDLK_r:
	  move("right");
	  break;
	case SDLK_e:
	  move("left");
	  break;
	/**
	 * Player 2 settings:
	 * T = UP; Y = DOWN; U = LEFT;I = RIGHT
	 * Z = ATTACK
	 * S = SECONDARY ATTACK
	 * */
	case SDLK_2:
	  player2.state = PLAYER_READY;
	  break;
	case SDLK_t:
	  move("up");
	  break;
	case SDLK_y:
	  move("down");
	  break;
	case SDLK_u:
	  move("right");
	  break;
	case SDLK_i:
	  move("left");
	  break;
	case SDLK_ESCAPE:
	  finishHim(app);
	  break;
  }
}

void handleArcadeKeyboard(App *app)
{
  SDL_Event event;
  if(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_KEYDOWN:
		handleArcadeKeys(app, &event.key.keysym.sym);
	}
  }
}

void handleMenu(App *app){
  handleArcadeKeyboard(app);
}

int hasNoReadyPlayers(Game *game) {
  return game->player1.state == PLAYER_READY || game->player2.state == PLAYER_READY;
}

int main( int argc, char* args[] )
{
	App app;
	Menu menu;
	app.state = STATE_MENU;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

	while(app.state != STATE_EXIT){
	  if (app.state == STATE_MENU){
		handleMenu(&app);
		if(hasNoReadyPlayers(&app.game)){
		  printf("No players at moment \n");
		}
	  }

	  app.screen = SDL_SetVideoMode( 1024, 768, 32, SDL_HWSURFACE);
	  SDL_FillRect(app.screen, NULL , 0xcccccccc);
	  SDL_UpdateRect(app.screen, 0, 0, 0, 0);
	}

	return 0;
}
