#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "font.h"

typedef struct {

} KeyboardConfig;

typedef struct {

} Menu;

typedef enum {
  PLAYER_READY,
  PLAYER_IDLE,
} PlayerStatus;

typedef struct{
  KeyboardddConfig config;
  PlayerStatus status;
} Player;

typedef struct {
  Player player1;
  Player player2;
} Game;

typedef enum {
  STATE_EXIT,
  STATE_MENU,
} AppState;

typedef struct {
  SDL_Surface *screen;
  Game game;
  AppState state;
  Menu menu;
} App;

void handleArcadeKeys(){

 case SDLK_ESCAPE:
		  case SDLK_q:
			app->state = STATE_EXIT;
			break;

}

void handleMenuKeyboard(App *app)
{
  SDL_Event event;
  if(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_KEYDOWN:
		handleArcadeKeys();
		switch(event.key.keysym.sym) {
		  handleArcadeKeys();
		 		}
	}
  }
}

void handleMenu(App *app){
  handleMenuKeyboardFor(app);
}

int hasNoReadyPlayers(Game *game) {
  return game->player1.status == PLAYER_READY || game->player2.status == PLAYER_READY;
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
		} else {

		}
	  }

	  app.screen = SDL_SetVideoMode( 1024, 768, 32, SDL_HWSURFACE);
	  SDL_FillRect(app.screen, NULL , 0xcccccccc);
	  SDL_UpdateRect(app.screen, 0, 0, 0, 0);
	}

	return 0;
}
