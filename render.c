#include <SDL_image.h>
#include "render.h"

void renderPlayer(SDL_Surface *screen, Player *player){
  if(player->state != PLAYER_READY) return;
  printf("blitting on X: %i/Y: %i\n", player->body.pos.x, player->body.pos.y);
  int a = player->body.angle;
  printf("angle %d\n", a);
  SDL_Surface *image;
  if(a >= 315 || a < 45) image = player->right;
  else if(a >= 45 && a < 135) image = player->down;
  else if(a >= 135 && a < 225) image = player->left;
  else if(a >= 225 && a < 315) image = player->up;
  else return;
  printf("%p %p %p %p %p\n", image, player->right, player->up, player->left, player->down);
  SDL_BlitSurface(image, NULL, screen, &player->body.pos);
}

void render(App *app){
  Game game = app->game;

  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_FillRect(app->screen, NULL , color);

  SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

  // renderPlayer(app->screen, &game.player1);
  //renderPlayer(app->screen, &game.player2);
  //SDL_UpdateRect(app->screen, 0, 0, 0, 0);
  SDL_Flip(app->screen);
}

void renderMapInit(App *app, int map_index) {
  char image_path[256];
  char hit_path[256];
  sprintf(image_path, "map%d.bmp", map_index);
  sprintf(hit_path, "map%d_hit.bmp", map_index);
  printf("map %d %s %s\n", map_index, image_path, hit_path);
  app->game.board.image = IMG_Load(image_path);
  app->game.board.hit = IMG_Load(hit_path);
  printf("%p %p\n", app->game.board.image, app->game.board.hit);

}

void renderInit(App *app){
  renderMapInit(app, 0);

  app->game.player1.up = IMG_Load("soldado1_costas.png");
  app->game.player1.down = IMG_Load("soldado1.png");
  app->game.player1.left = IMG_Load("soldado1.png");
  app->game.player1.right = IMG_Load("soldado1_costas.png");
  app->game.player2.up = IMG_Load("engenheiro1.png");
  app->game.player2.down = IMG_Load("engenheiro1.png");
  app->game.player2.left = IMG_Load("engenheiro1.png");
  app->game.player2.right = IMG_Load("engenheiro1.png");

  app->game.player1.state = PLAYER_IDLE;
  app->game.player2.state = PLAYER_IDLE;

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







