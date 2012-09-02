#ifndef _APP_H
#define _APP_H
#include <SDL.h>

#define ENEMY_COUNT 1
#include "aStarLibrary.h"

typedef enum {
  MENU_RESUME = 0,
  MENU_NEW_GAME,
  MENU_CREDITS,
  MENU_QUIT,
  MENU_COUNT
} MenuItem;

typedef struct {
  MenuItem selected;
  SDL_Surface *soldier;
  SDL_Surface *zombie;
  SDL_Surface *engineer;
  SDL_Surface *bigZombie;
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

typedef enum {
  ENEMY_LIVE,
  ENEMY_DEAD,
} EnemyState;

typedef struct{
  SDL_Surface *image;
} EnemyClass;

typedef struct{
  Body body;
  SDL_Surface *image;
  EnemyState state;
} Enemy;


typedef struct {
	int x,y;
	int open;
} Spawn;

typedef struct {
	SDL_Rect rect;
	SDL_Surface *image;
} Sprite;

typedef struct{
  SDL_Surface *image;
  SDL_Surface *hit;
  char wall[mapWidth][mapHeight];
  char crowd[mapWidth][mapHeight];
  char powerup[mapWidth][mapHeight];
  Spawn spawn[mapWidth*mapHeight];
  int spawn_count;
} Board;

typedef struct {
  Player player1;
  Player player2;
  Enemy enemies[ENEMY_COUNT];
  Board board;
  Uint32 start;  
  Uint32 spawnTime;  
  EnemyClass enemy_class_medic;
  EnemyClass enemy_class_soldier;
} Game;

typedef enum {
  STATE_MENU,
  STATE_PAUSED,
  STATE_PLAYING,
  STATE_CREDITS,
  STATE_EXIT,
} AppState;

typedef struct {
  SDL_Surface *screen;
  Game game;
  int debug;
  AppState state;
  /**
   * just to handle if is to show or not the resume page
   * TODO: should be refactored to not use this attr
   */
  AppState stateBeforeCredits;
  Menu menu;
} App;

#endif
