#include "render.h"

void render(App *app){
  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_FillRect(app->screen, NULL , color);
  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderInit(App *app){
  app->screen = SDL_SetVideoMode( 1024, 768, 32, SDL_HWSURFACE);
}
