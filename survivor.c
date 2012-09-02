#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "font.h"
#include "app.h"
#include "render.h"


void angle_rotate(int *a0_base, int a1, float f)
{
    int a0 = *a0_base;
    if(fabs(a1 - a0) > 180) {
        if(a0 < a1)
            a0 += 360;
        else
            a1 += 360;
    }
    *a0_base = (int)((720+a0)*(1-f) + f*(720+a1)) % 360;
}

void body_move(Game *game, Body *body, int angle)
{
    if(body->action == ACTION_DEATH){
	  return;
	}
	printf("body: angle: %i\n", body->angle);

    float v = body->max_vel;

    angle_rotate(&body->angle, angle, body->ang_vel);
    float a = body->angle * M_PI / 180;
    body->pos.x += cos(a) * v;
    body->pos.y -= sin(a) * v;
    //body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;
}

void finishHim(App *app){
  app->state = STATE_EXIT;
}

void handleArcadeKeysDown(App *app, SDLKey *key){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;
  switch(*key){
	case SDLK_1:
	  player1->state = PLAYER_READY;
	  break;
	case SDLK_2:
	  player2->state = PLAYER_READY;
	  break;
	case SDLK_ESCAPE:
	  finishHim(app);
	  break;
  }

 }
/**
 * Keystate is responsible to handle pressed keys
 */
void handleArcadeKeystate(App *app){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;

  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);

  /**
   * Player 1 settings:
   * Q = UP; W = DOWN; E = LEFT; R = RIGHT
   * A = ATTACK
   * S = SECONDARY ATTACK
   * */

  if (keystate[SDLK_w] ) {
	body_move(&app->game, &player1->body, 90);
  }
  if (keystate[SDLK_d] ) {
	body_move(&app->game, &player1->body, 0);
  }
  if (keystate[SDLK_s] ) {
	body_move(&app->game, &player1->body, 270);
  }
  if (keystate[SDLK_a] ) {
	body_move(&app->game, &player1->body, 180);
  }
  /**
   * Player 2 settings:
   * T = UP; Y = DOWN; U = LEFT;I = RIGHT
   * Z = ATTACK
   * S = SECONDARY ATTACK
   * */
  if (keystate[SDLK_UP]) {
	body_move(&app->game, &player2->body, 90);
  }
  if (keystate[SDLK_RIGHT] ) {
	body_move(&app->game, &player2->body, 0);
  }
  if (keystate[SDLK_DOWN] ) {
	body_move(&app->game, &player2->body, 270);
  }
  if (keystate[SDLK_LEFT] ) {
	body_move(&app->game, &player2->body, 180);
  }
}

void handleArcadeKeyboard(App *app)
{
  SDL_Event event;
  while(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_KEYDOWN:
		handleArcadeKeysDown(app, &event.key.keysym.sym);
	}
  }
  handleArcadeKeystate(app);
}

void handleMenu(App *app){
  handleArcadeKeyboard(app);
}

int hasNoReadyPlayers(Game *game) {
  return !game->player1.state == PLAYER_READY && !game->player2.state == PLAYER_READY;
}

void gameInit(App *app){

  app->game.player1.state = PLAYER_IDLE;

  /**
   * Player 1 init settings
   * */
  Body *p1body = &app->game.player1.body;
  p1body->ang_vel = 0.3;
  p1body->max_vel = 5;
  p1body->angle = 1;
  p1body->pos.x = 1204/2+15;
  p1body->pos.y = 768/2+15;

  /**
   * Player 2 init settings
   * */
  Body *p2body = &app->game.player2.body;

  p2body->ang_vel = 0.3;
  p2body->max_vel = 5;
  p2body->angle = 1;
  p2body->pos.x = 1204/2+40;
  p2body->pos.y = 768/2+40;

}

int main( int argc, char* args[] )
{
	App app;
	Menu menu;
	app.state = STATE_MENU;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

	renderInit(&app);
	gameInit(&app);
	while(app.state != STATE_EXIT){
	  if (app.state == STATE_MENU){
		handleMenu(&app);
		if(hasNoReadyPlayers(&app.game)){
		  printf("No players at moment \n");
		}
	  }
	  render(&app);
	}

	return 0;
}


