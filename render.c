#include "render.h"
#include "font.h"

void renderPlayer(SDL_Surface *screen, Player *player){
  if(player->state == PLAYER_READY){
	printf("blitting on X: %i/Y: %i\n", player->body.pos.x, player->body.pos.y);
	SDL_BlitSurface(player->image, NULL, screen, &player->body.pos);
  }
}

void render(App *app){
  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_FillRect(app->screen, NULL , color);
  Game game = app->game;
  renderPlayer(app->screen, &game.player1);
  renderPlayer(app->screen, &game.player2);
  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderInit(App *app){
  app->menu.pointerImage = SDL_LoadBMP("red.bmp");
  app->game.player1.image = SDL_LoadBMP("green.bmp");
  app->game.player2.image = SDL_LoadBMP("red.bmp");

  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
}

void renderCredits(App *app){

}

/**
 *
 * TODO: refactor this to be part of render func
 */
void renderMenu(App *app){
  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_FillRect(app->screen, NULL , color);
  Menu *menu = &app->menu;
  SDL_Surface *screen = app->screen;

  text_write(screen, 100, 250, "new game", menu->selected == MENU_NEW_GAME);
  text_write(screen, 100, 450, "credits", menu->selected == MENU_CREDITS);
  text_write(screen, 100, 550, "exit", menu->selected == MENU_QUIT);

  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}









