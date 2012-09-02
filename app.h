#ifndef _APP_H
#define _APP_H
#include <SDL.h>

typedef enum {
  MENU_NEW_GAME = 0,
  MENU_CREDITS,
  MENU_QUIT,
  MENU_COUNT
} MenuItem;

typedef struct {
  MenuItem selected;
  SDL_Surface *pointerImage;
  SDL_Surface *background;
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
  float angle; // degree
} Body;

typedef struct{
  PlayerStatus state;
  Body body;
  SDL_Surface *left;
  SDL_Surface *right;
  SDL_Surface *up;
  SDL_Surface *down;
} Player;

typedef struct{
  SDL_Surface *image;
  SDL_Surface *hit;
} Board;

typedef struct {
  Player player1;
  Player player2;
  Board board;
} Game;

typedef enum {
  STATE_MENU,
  STATE_PLAYING,
  STATE_CREDITS,
  STATE_EXIT,
} AppState;

typedef struct {
  SDL_Surface *screen;
  Game game;
  AppState state;
  Menu menu;
} App;

#endif
