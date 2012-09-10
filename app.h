#ifndef _APP_H
#define _APP_H
#include <SDL.h>
#include <SDL_mixer.h>

#define ENEMY_COUNT 666
#define POWERUP_COUNT 32
#define PLAYER_COUNT 2
#define SHOT_COUNT 666
#define SPRITE_COUNT (PLAYER_COUNT+ENEMY_COUNT+SHOT_COUNT+POWERUP_COUNT)
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
  BODY_ALIVE = 0,
  BODY_IDLE,
  BODY_DEAD
} BodyStatus;

typedef enum {
  PLAYER_IDLE = 0,
  PLAYER_DEAD,
  PLAYER_READY,
} PlayerStatus;

typedef struct {
  int spread;
  int freq;
  SDL_Surface *shot_image;
  SDL_Surface *hit_image;
  SDL_Surface *image;
  Mix_Chunk *sound;
  float damage;
  float range;
  int ammo_total;
} ItemType;

typedef struct {
  ItemType *type;
  int ammo_used;
  int should_show;
  int x;
  int y;
} Item;

typedef struct {
  SDL_Rect pos;
  float life;
  int ammo;
  Item item;
  BodyStatus status;
  float max_vel;
  float ang_vel;
  float angle; // degree
  Mix_Chunk *onHitSound;
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
  int pathfinder;
  int pathfinder_other;
  Body *target;
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
  int wall[mapWidth][mapHeight];
  int air[mapWidth][mapHeight];
  int crowd[mapWidth][mapHeight];
  int hittable[mapWidth][mapHeight];
  int powerup[mapWidth][mapHeight];
  int spawn_map[mapWidth][mapHeight];
  Spawn spawn[mapWidth*mapHeight];
  int spawn_count;
  Sprite sprite[SPRITE_COUNT];
  int sprite_count;
  Item powerups[POWERUP_COUNT];
} Board;

typedef struct {
  SDL_Surface *image;
} HealthPack;

enum {
	ITEM_HEALTH_PACK,
	ITEM_PLAYER_BULLET,
	ITEM_PLAYER_FLAME,
	ITEM_PLAYER_COUNT,
	ITEM_ENEMY_MEDIC,
	ITEM_COUNT
};
enum {
  ENEMY_MEDIC,
  ENEMY_SOLDIER,
  ENEMY_TYPE_COUNT
};

typedef struct {
  Player player1;
  Player player2;
  Enemy enemies[ENEMY_COUNT];
  Board board;
  Uint32 start;
  Uint32 spawnTime;
  int latest_enemy_updated;
  int kill_count;
  HealthPack health_pack;
  ItemType itemtype[ITEM_COUNT];
  EnemyClass enemy_class[ENEMY_TYPE_COUNT];
} Game;

typedef enum {
  STATE_MENU,
  STATE_PAUSED,
  STATE_PLAYING,
  STATE_GAMEOVER,
  STATE_CREDITS,
  STATE_EXIT,
} AppState;

typedef enum {
  CREDITS_TEAM = 0,
  CREDITS_SOUND
} Credits;

typedef enum {
	DEBUG_NONE,
	DEBUG_WALL,
	DEBUG_AIR,
	DEBUG_AI,
	DEBUG_ENEMY,
	DEBUG_MOVE,
	DEBUG_SHOT,
	DEBUG_ITEM,
	DEBUG_COUNT
} Debug;

typedef struct {
  SDL_Surface *screen;
  Game game;
  Credits credits;
  Debug debug;
  AppState state;
  /**
   * just to handle if is to show or not the resume page
   * TODO: should be refactored to not use this attr
   */
  AppState stateBeforeCredits;
  SDL_Surface *logo;
  Menu menu;
} App;

int hit(App *app, Body *source, Body *target);

#endif
