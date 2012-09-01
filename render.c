#include "render.h"

void render(App *app){
  SDL_FillRect(app->screen, NULL , 0xcccccccc);
  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderInit(App *app){
  app->screen = SDL_SetVideoMode( 1024, 768, 32, SDL_HWSURFACE);
}
