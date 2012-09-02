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
	printf("body: angle: %i", body->angle);

    float v = body->max_vel;

    angle_rotate(&body->angle, angle, body->ang_vel);
    float a = body->angle * M_PI / 180;
    body->pos.x += cos(a) * v;
    body->pos.y -= sin(a) * v;
	printf("\nsetting position x: %i y %i\n", body->pos.x);
	printf("\nsetting position x: %i y %i\n", body->pos.y);
    //body->frame = (body->frame+(rand()%2)) % body->sprite->frame_count;
}

void finishHim(App *app){
  app->state = STATE_EXIT;
}

void handleArcadeKeys(App *app, SDLKey *key){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;
  printf("ola");
  switch(*key){
	/**
	 * Player 1 settings:
	 * Q = UP; W = DOWN; E = LEFT; R = RIGHT
	 * A = ATTACK
	 * S = SECONDARY ATTACK
	 * */
	case SDLK_1:
	  printf("player 1 ready\n");
	  player1->state = PLAYER_READY;
	  break;
	case SDLK_w:
	  body_move(&app->game, &player1->body, 0);
	  break;
	case SDLK_s:
	  body_move(&app->game, &player1->body, 180);
	  break;
	case SDLK_a:
	  body_move(&app->game, &player1->body, 270);
	  break;
	case SDLK_d:
	  body_move(&app->game, &player1->body, 90);
	  break;
	/**
	 * Player 2 settings:
	 * T = UP; Y = DOWN; U = LEFT;I = RIGHT
	 * Z = ATTACK
	 * S = SECONDARY ATTACK
	 * */
	case SDLK_2:
	  player2->state = PLAYER_READY;
	  break;
	case SDLK_t:
	  body_move(&app->game, &player2->body, 0);
	  break;
	case SDLK_y:
	  body_move(&app->game, &player2->body, 90);
	  break;
	case SDLK_u:
	  body_move(&app->game, &player2->body, 45);
	  break;
	case SDLK_i:
	  body_move(&app->game, &player2->body, 180);
	  break;
	case SDLK_ESCAPE:
	  finishHim(app);
	  break;
  }
}

void handleArcadeKeyboard(App *app)
{
  SDL_Event event;
  while(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_KEYDOWN:
		handleArcadeKeys(app, &event.key.keysym.sym);
	}
  }
}

void handleMenu(App *app){
  handleArcadeKeyboard(app);
}

int hasNoReadyPlayers(Game *game) {
  return !game->player1.state == PLAYER_READY && !game->player2.state == PLAYER_READY;
}

int main( int argc, char* args[] )
{
	App app;
	Menu menu;
	app.state = STATE_MENU;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;

	renderInit(&app);
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


