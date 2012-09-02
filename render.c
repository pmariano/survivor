#include "render.h"

void renderPlayer(SDL_Surface *screen, Player *player){
  printf("blitting on X: %i/Y: %i\n", player->position.x, player->position.y);
  SDL_BlitSurface(player->image, NULL, screen, &player->position);
}

void render(App *app){
  Game game = app->game;

  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_FillRect(app->screen, NULL , color);

  SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

  renderPlayer(app->screen, &game.player1);
  renderPlayer(app->screen, &game.player2);
  // SDL_UpdateRect(app->screen, 0, 0, 0, 0);
  SDL_Flip(app->screen);
}

void renderInit(App *app){
  app->game.player1.image = SDL_LoadBMP("green.bmp");
  app->game.player2.image = SDL_LoadBMP("red.bmp");

  app->game.board.image = SDL_LoadBMP("map0.bmp");
  app->game.board.hit = SDL_LoadBMP("hit0.bmp");

  app->game.player1.position.x = 1204/2+15;
  app->game.player1.position.y = 768/2+15;
  app->game.player2.position.x = 1204/2+40;
  app->game.player2.position.y = 768/2+40;

  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
}
