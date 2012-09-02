#ifndef _APP_H
#define _APP_H
#include <SDL.h>
typedef struct {

} Menu;

typedef enum {
    ACTION_MOVE=0,
    ACTION_ATTACK,
    ACTION_DEATH,
    ACTION_COUNT
} Action;

typedef enum {
  PLAYER_IDLE,
  PLAYER_READY,
} PlayerStatus;

typedef struct {
  SDL_Rect pos;
  Action action;
  float max_vel;
  float ang_vel;
  int angle; // degree
} Body;

typedef struct{
  PlayerStatus state;
  Body body;
  SDL_Surface *image;
} Player;

typedef struct {
  Player player1;
  Player player2;
} Game;

typedef enum {
  STATE_MENU,
  STATE_EXIT,
} AppState;

typedef struct {
  SDL_Surface *screen;
  Game game;
  AppState state;
  Menu menu;
} App;

#endif
