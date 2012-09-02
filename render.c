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
  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderInit(App *app){
  app->game.player1.image = SDL_LoadBMP("green.bmp");
  app->game.player2.image = SDL_LoadBMP("red.bmp");
  app->game.player1.state = PLAYER_IDLE;

  Body *p1body = &app->game.player1.body;
  Body *p2body = &app->game.player2.body;
  p1body->ang_vel = 1;
  p1body->max_vel = 2;
  p1body->angle = 1;

  p1body->pos.x = 1204/2+15;
  p1body->pos.y = 768/2+15;

  p2body->pos.x = 1204/2+40;
  p2body->pos.y = 768/2+40;

  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
}







