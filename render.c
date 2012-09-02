#include "render.h"

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
  app->game.player1.image = SDL_LoadBMP("green.bmp");
  app->game.player2.image = SDL_LoadBMP("red.bmp");

  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
}







