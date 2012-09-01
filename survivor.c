#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "font.h"


typedef struct {

} Game;

typedef struct {

} Menu;

typedef enum {
  STATE_EXIT,
  STATE_MENU,
} AppState;

typedef struct {
  SDL_Surface *screen;
  Game *game;
  AppState state;
  Menu *menu;
} App;

void handleKeyboard(App *app)
{
  SDL_Event event;
  if(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_QUIT:
		app->state = STATE_EXIT;
		break;
	  case SDL_KEYDOWN:
		switch(event.key.keysym.sym) {
		  case SDLK_ESCAPE:
		  case SDLK_q:
			app->state = STATE_EXIT;
			break;

		}
	}
  }
}

void handleMenu(){



}

int main( int argc, char* args[] )
{
	App app;
	Menu menu;
	app.state = STATE_MENU;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

	while(app.state != STATE_EXIT){
	  if (app.state == STATE_MENU){
		handleMenu();
		handleKeyboard(&app);
	  }

	  app.screen = SDL_SetVideoMode( 1024, 768, 32, SDL_HWSURFACE);
	  SDL_FillRect(app.screen, NULL , 0x00000000);
	  SDL_UpdateRect(app.screen, 0, 0, 0, 0);
	}

	return 0;
}
