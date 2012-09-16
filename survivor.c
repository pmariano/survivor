#include <SDL.h>
#include <SDL_keysym.h>
#include <SDL_image.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>

#include "font.h"
#include "app.h"
#include "sound.h"
#include "render.h"
#include "movement.h"
#include "aStarLibrary.h"

#define FPS 40
#define MAX(a,b) ((a)>(b)?(a):(b))

void finishHim(App *app){
  app->state = STATE_EXIT;
}

void checkGameover(App *app){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;

  int numCurrentPlayers = (player1->body.status != BODY_DEAD) + (player2->body.status != BODY_DEAD);
  if(!numCurrentPlayers){
	app->state = STATE_GAMEOVER;
  }
}

void gameInit(App *app){
	app->game.start = SDL_GetTicks();
	app->game.spawnTime = app->game.start;
	app->game.kill_count= 0;

	/**
	 * Player 1 init settings
	 * */
	Body *p1body = &app->game.player1.body;
	app->game.player1.body.status = BODY_DEAD;
	p1body->ang_vel = 0.04;
	p1body->max_vel = 4;
	p1body->angle = 0;
	p1body->life = 100.0;
	p1body->item.type = &app->game.itemtype[ITEM_PLAYER_BULLET];
	p1body->item.ammo_used = 0 ;
	p1body->onHitSound = Mix_LoadWAV("sounds/ouch.wav");
	p1body->status = BODY_DEAD;
	player_spawn_pos(&app->game, &p1body->pos.x, &p1body->pos.y);

	/**
	 * Player 2 init settings
	 * */
	Body *p2body = &app->game.player2.body;
	app->game.player2.body.status = BODY_DEAD;

	p2body->ang_vel = 0.4;
	p2body->max_vel = 6;
	p2body->angle = 1;
	p2body->life = 100.0;
	p2body->item.type = &app->game.itemtype[ITEM_PLAYER_BULLET];
	p2body->item.ammo_used = 0 ;
	p2body->status = BODY_DEAD;
	p2body->onHitSound = Mix_LoadWAV("sounds/ouch.wav");
	player_spawn_pos(&app->game, &p2body->pos.x, &p2body->pos.y);

	app->game.latest_enemy_updated = 0;

	app->credits = 0;
  int i;
  for(i=0;i < ENEMY_COUNT; i++)
  {
	app->game.enemies[i].body.status = BODY_DEAD;
  }

  for(i=0;i<POWERUP_COUNT; i++)
  {
	app->game.board.powerups[i].should_show = 0;
  }
}

void resetApp(App *app){
  gameInit(app);
  //reset here other stuffs
}

void pauseOrJoinTheGame(App *app, Player *player){
  if(player->body.status == BODY_ALIVE){
	app->state = STATE_PAUSED;
	app->menu.selected = MENU_RESUME;
  } else{
	player->body.status = BODY_ALIVE;
	player->body.status = BODY_ALIVE;
  }
}

void bindGameplayKeysDown(App *app, SDLKey *key){
	Player *player1 = &app->game.player1;
	Player *player2 = &app->game.player2;
	SDLMod mod = SDL_GetModState();

	switch(*key){
		case SDLK_1:
			pauseOrJoinTheGame(app, player1);
			break;
		case SDLK_2:
			pauseOrJoinTheGame(app, player2);
			break;
		case SDLK_0:
			app->debug = (DEBUG_COUNT + app->debug + (mod & KMOD_SHIFT ? -1 : 1)) % DEBUG_COUNT;
			break;
		case SDLK_ESCAPE:
			app->state = STATE_PAUSED;
			app->menu.selected = MENU_RESUME;
			break;
		case SDLK_s:
			grab(app, &player1->body);
			break;
		case SDLK_x:
			grab(app, &player2->body);
			break;
	}
}

void bindMenuKeysDown(App *app, SDLKey *key){
  Player *player1 = &app->game.player1;
  Player *player2 = &app->game.player2;
  Menu *menu = &app->menu;
  int firstMenu = MENU_NEW_GAME;

  /**
   * when game is paused there is more menu options
   * */
  if(app->state == STATE_PAUSED){
	firstMenu = MENU_RESUME;
  }

  switch(*key){
	case SDLK_1:
	  player1->body.status = BODY_ALIVE;
	  break;
	case SDLK_2:
	  player2->body.status = BODY_ALIVE;
	  break;
	case SDLK_UP:
	case SDLK_q:
	case SDLK_t:
	  if(menu->selected != firstMenu){
		menu->selected--;
	  }
	  break;
	case SDLK_DOWN:
	case SDLK_w:
	case SDLK_y:
	  if(menu->selected < MENU_COUNT - 1){
		menu->selected++;
	  }
	  break;
	case SDLK_a:
	case SDLK_z:
	case SDLK_RETURN:
	  if(app->state == STATE_CREDITS){
		if(app->credits == CREDITS_SOUND){
		  app->state = app->stateBeforeCredits;
		  app->credits = CREDITS_TEAM;
		} else{
		  app->credits = CREDITS_SOUND;
		}
		break;
	  }

	  if(menu->selected == MENU_NEW_GAME){
		resetApp(app);
		app->state = STATE_PLAYING;
	  } else if (menu->selected == MENU_QUIT){
		finishHim(app);
	  } else if(menu->selected == MENU_CREDITS){
		AppState s = app->state;
		app->stateBeforeCredits = s;
		app->state = STATE_CREDITS;
	  } else if(menu->selected == MENU_RESUME){
		app->state = STATE_PLAYING;
	  }
	  if(*key == SDLK_z){
		player2->body.status = BODY_ALIVE;
		player2->body.status = BODY_ALIVE;
	  } else {
		printf("player 1 is ready\n");
		player1->body.status = BODY_ALIVE;
		player1->body.status = BODY_ALIVE;
	  }
	  break;
	case SDLK_ESCAPE:
	  finishHim(app);
	  break;
  }
}

/**
 * Keystate is responsible to handle pressed keys
 */
void bindGameplayKeystate(App *app){
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

  player_move(app, &player1->body,
	  keystate[SDLK_UP] || keystate[SDLK_q],
	  keystate[SDLK_RIGHT] || keystate[SDLK_r],
	  keystate[SDLK_DOWN] || keystate[SDLK_w],
	  keystate[SDLK_LEFT] || keystate[SDLK_e],
	  keystate[SDLK_s]
	  );

  /**
   * Player 2 settings:
   * T = UP; Y = DOWN; U = LEFT;I = RIGHT
   * Z = ATTACK
   * S = SECONDARY ATTACK
   * */
  player_move(&app->game, &player2->body,
	  keystate[SDLK_KP6] || keystate[SDLK_i],
	  keystate[SDLK_KP8] || keystate[SDLK_t],
	  keystate[SDLK_KP4] || keystate[SDLK_u],
	  keystate[SDLK_KP5] || keystate[SDLK_KP2] || keystate[SDLK_y],
	  keystate[SDLK_x]
	  );

  if(keystate[SDLK_a])
	shoot(app, &player1->body);
  if(keystate[SDLK_z])
	shoot(app, &player2->body);
}

void bindKeyboard(App *app)
{
  SDL_Event event;
  while(SDL_PollEvent(&event)){
	switch(event.type) {
	  case SDL_KEYDOWN:
		if (app->state == STATE_PLAYING){
		  bindGameplayKeysDown(app, &event.key.keysym.sym);
		} else{
		  bindMenuKeysDown(app, &event.key.keysym.sym);
		}
	}
  }

  if (app->state == STATE_PLAYING){
	bindGameplayKeystate(app);
  }
}

void handleDelay(Uint32 start) {
  Uint32 end = SDL_GetTicks();
  int actual_delta = end - start;
  int expected_delta = 1000/FPS;
  int delay = MAX(0, expected_delta - actual_delta);
  SDL_Delay(delay);
}

void spawnEnemy(App *app)
{
  Game *game = &app->game;
  Enemy *enemy = NULL;
  int x,y;

  int i;
  for(i = 0; i < ENEMY_COUNT; i++)
  {
	if(game->enemies[i].body.status == BODY_DEAD)
	{
	  enemy = &game->enemies[i];
	  break;
	}
  }

  if(enemy != NULL && enemy_spawn_pos(game, &x,&y))
  {
	// printf("spawn %d\n", i);
	enemy->body.status = BODY_ALIVE;
	int k = rand() % ENEMY_TYPE_COUNT;
	enemy->image = game->enemy_class[k].image;
	Body *enemybody = &enemy->body;
	enemybody->life = game->enemy_class[k].max_life;
	enemybody->item.type = game->enemy_class[k].type;
	enemybody->ang_vel = 0.05;
	enemybody->max_vel = 2.5;
	enemybody->angle = 0;
	enemybody->pos.x = x;
	enemybody->pos.y = y;
  }
}

void loadMap(App *app, int map_index) {
  char image_path[256];
  char hit_path[256];
  sprintf(image_path, "data/map%d.png", map_index);
  sprintf(hit_path, "data/map%d_hit.png", map_index);
  app->game.board.image = IMG_Load(image_path);
  app->game.board.hit = IMG_Load(hit_path);
  moveInit(app);
}

void loadItems(App *app) {
	app->game.itemtype[ITEM_ENEMY_MEDIC].damage = 1;
	app->game.itemtype[ITEM_ENEMY_MEDIC].score = 1;
	app->game.itemtype[ITEM_ENEMY_MEDIC].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_ENEMY_MEDIC].sound = Mix_LoadWAV("sounds/ouch.wav");
	app->game.itemtype[ITEM_ENEMY_SOLDIER].damage = 4;
	app->game.itemtype[ITEM_ENEMY_SOLDIER].score = 5;
	app->game.itemtype[ITEM_ENEMY_SOLDIER].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_ENEMY_SOLDIER].sound = Mix_LoadWAV("sounds/ouch.wav");
	app->game.itemtype[ITEM_PLAYER_BULLET].damage = 75;
	app->game.itemtype[ITEM_PLAYER_BULLET].range = 1024;
	app->game.itemtype[ITEM_PLAYER_BULLET].hit_image = IMG_Load("data/bullet_hit.png");
	app->game.itemtype[ITEM_PLAYER_BULLET].image = IMG_Load("data/bullet_ammo.png");
	app->game.itemtype[ITEM_PLAYER_BULLET].shot_image = IMG_Load("data/bullet_shot.png");
	app->game.itemtype[ITEM_PLAYER_BULLET].freq = 8;
	app->game.itemtype[ITEM_PLAYER_BULLET].spread = 3;
	app->game.itemtype[ITEM_PLAYER_BULLET].ammo_total = 1000;
	app->game.itemtype[ITEM_PLAYER_BULLET].sound = Mix_LoadWAV("sounds/machinegun.wav");
	app->game.itemtype[ITEM_PLAYER_FLAME].damage = 250;
	app->game.itemtype[ITEM_PLAYER_FLAME].range = 150;
	app->game.itemtype[ITEM_PLAYER_FLAME].image = IMG_Load("data/fire_ammo.png");
	app->game.itemtype[ITEM_PLAYER_FLAME].hit_image = IMG_Load("data/fire_hit.png");
	app->game.itemtype[ITEM_PLAYER_FLAME].shot_image = IMG_Load("data/fire_shot.png");
	app->game.itemtype[ITEM_PLAYER_FLAME].spread = 20;
	app->game.itemtype[ITEM_PLAYER_FLAME].sound = Mix_LoadWAV("sounds/flame.wav");
	app->game.itemtype[ITEM_PLAYER_FLAME].freq = 2;
	app->game.itemtype[ITEM_PLAYER_FLAME].ammo_total = 250;
	app->game.itemtype[ITEM_HEALTH_PACK].damage = -50;
	app->game.itemtype[ITEM_HEALTH_PACK].image = IMG_Load("data/health.png");
}

void loadEnemies(App *app) {
  app->game.enemy_class[ENEMY_MEDIC].image = IMG_Load("data/zombie2.png");
  app->game.enemy_class[ENEMY_MEDIC].type = &app->game.itemtype[ITEM_ENEMY_MEDIC];
  app->game.enemy_class[ENEMY_MEDIC].max_life = 100;
  app->game.enemy_class[ENEMY_SOLDIER].image = IMG_Load("data/zombie1.png");
  app->game.enemy_class[ENEMY_SOLDIER].type = &app->game.itemtype[ITEM_ENEMY_SOLDIER];
  app->game.enemy_class[ENEMY_SOLDIER].max_life = 500;
}


int grab(App *app, Body *body)
{
	int x = body->pos.x/tileSize;
	int y = body->pos.y/tileSize;
	int i = app->game.board.powerup[x][y];
	if(i && app->game.board.powerups[--i].should_show) {
		if(app->game.board.powerups[i].type->damage < 0) {
			body->life -= app->game.board.powerups[i].type->damage;
		} else  {
			body->item = app->game.board.powerups[i];
		}
		app->game.board.powerups[i].should_show = 0;
		app->game.board.powerup[x][y] = 0;
	}
}

int hit(App *app, Body *source, Body *target){
	target->life -= source->item.type->damage;

	if(source->item.type->hit_image) {
		SDL_Rect rect = {
			target->pos.x - source->item.type->hit_image->w/2,
			target->pos.y - source->item.type->hit_image->h/2,
			source->item.type->hit_image->w,
			source->item.type->hit_image->h
		};
		SDL_BlitSurface(source->item.type->hit_image, NULL, app->screen, &rect);
		// printf("splash %d %d\n", target->pos.x, target->pos.y);
	}

	if(target->status != BODY_DEAD){
	  playSound(target->onHitSound);
	}

  if(target->life <= 0){
	if(target->status == BODY_ALIVE){
		if(!app->debug || target->item.type->score) { // player immortal on debug
			app->game.kill_count += !!target->item.type->score;
			target->status = BODY_DEAD;
		}
	}
	return 1;
  }
  return 0;
}

int draw(App *app, Body *body, int x, int y)
{
	if(x >= 0 && x < app->screen->w && y >= 0 && y < app->screen->h) {
#if 0
		Uint8 *p = ((Uint8*)app->screen->pixels) + (x*app->screen->format->BytesPerPixel+y*app->screen->pitch);
		p[0] = 0xff;
		p[1] = 0xff;
#else
		if(body->item.type->shot_image && (rand() % (body->item.type->freq)) == 0) {
			x += (rand() % (body->item.type->spread/2+1)) - body->item.type->spread/4;
			y += (rand() % (body->item.type->spread/2+1)) - body->item.type->spread/4;
			SDL_Rect rect = {
				x - body->item.type->shot_image->w/2,
				y - body->item.type->shot_image->h/2,
				body->item.type->shot_image->w,
				body->item.type->shot_image->h
			};
			SDL_BlitSurface(body->item.type->shot_image, NULL, app->screen, &rect);
		}
#endif
	}
	int target = is_air(&app->game, body, x, y);
	if(target) {
		if(target>=4) {
			int i = target - 4;
			hit(app, body, &app->game.enemies[i].body);
		}
	}
  return target;
}

int aimer(App *app, Body *body, int x, int y)
{
	if(x >= 0 && x < app->screen->w && y >= 0 && y < app->screen->h) {
#if 0
		Uint8 *p = ((Uint8*)app->screen->pixels) + (x*app->screen->format->BytesPerPixel+y*app->screen->pitch);
		p[0] = 0xff;
		p[1] = 0xff;
#else
    SDL_Rect rect = {
      x - app->game.itemtype[ITEM_PLAYER_BULLET].shot_image->w/2,
      y - app->game.itemtype[ITEM_PLAYER_BULLET].shot_image->h/2,
      app->game.itemtype[ITEM_PLAYER_BULLET].shot_image->w,
      app->game.itemtype[ITEM_PLAYER_BULLET].shot_image->h
    };
    SDL_BlitSurface(app->game.itemtype[ITEM_PLAYER_BULLET].shot_image, NULL, app->screen, &rect);
#endif
	}
  return 0;
}

int aim(App *app, Body *body)
{
  Uint8 *keystate;
  keystate = SDL_GetKeyState(NULL);
  if(keystate[SDLK_a] || keystate[SDLK_z] ) return;
	int x1, y1, x2, y2;
	int dx, dy, i, e;
	int incx, incy, inc1, inc2;
	int x,y;
	int range;
	if(body->status != BODY_ALIVE)
		return;

	range = 150;

	x1 = body->pos.x;
	y1 = body->pos.y;

	float a = (int)(body->angle + ((rand() % (body->item.type->spread+1)) - body->item.type->spread/2))%360;
	x2 = x1 + cos(a * M_PI / 180.) * range;
	y2 = y1 - sin(a * M_PI / 180.) * range;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if(dx > dy)
	{
	  if(aimer(app,body,x,y)) return 1;
      e = 2*dy - dx;
      inc1 = 2*( dy -dx);
      inc2 = 2*dy;
      for(i = 0; i < dx; i++)
      {
        if(e >= 0)
        {
          y += incy;
          e += inc1;
        } else e += inc2;
        x += incx;
        if(aimer(app,body,x,y)) return 1;
      }
  }
  else
  {
    if(aimer(app,body,x,y)) return 1;
      e = 2*dx - dy;
      inc1 = 2*( dx - dy);
      inc2 = 2*dx;
      for(i = 0; i < dy; i++)
      {
        if(e >= 0)
        {
          x += incx;
          e += inc1;
        }
        else e += inc2;
        y += incy;
        if(aimer(app,body,x,y)) return 1;
      }
  }
  return 0;
}

int shoot(App *app, Body *body)
{
	int x1, y1, x2, y2;
	int dx, dy, i, e;
	int incx, incy, inc1, inc2;
	int x,y;
	int range;
	if(body->status != BODY_ALIVE)
		return;
	if(!app->debug && ++body->item.ammo_used > body->item.type->ammo_total) // infinit ammo on debug
		return;

	range = body->item.type->range;
	playSound(body->item.type->sound, -1);

	x1 = body->pos.x;
	y1 = body->pos.y;

	float a = (int)(body->angle + ((rand() % (body->item.type->spread+1)) - body->item.type->spread/2))%360;
	x2 = x1 + cos(a * M_PI / 180.) * range;
	y2 = y1 - sin(a * M_PI / 180.) * range;

	dx = x2 - x1;
	dy = y2 - y1;

	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;
	incx = 1;
	if(x2 < x1) incx = -1;
	incy = 1;
	if(y2 < y1) incy = -1;
	x=x1;
	y=y1;

	if(dx > dy)
	{
	  if(draw(app,body,x,y)) return 1;
      e = 2*dy - dx;
      inc1 = 2*( dy -dx);
      inc2 = 2*dy;
      for(i = 0; i < dx; i++)
      {
        if(e >= 0)
        {
          y += incy;
          e += inc1;
        } else e += inc2;
        x += incx;
        if(draw(app,body,x,y)) return 1;
      }
  }
  else
  {
    if(draw(app,body,x,y)) return 1;
      e = 2*dx - dy;
      inc1 = 2*( dx - dy);
      inc2 = 2*dx;
      for(i = 0; i < dy; i++)
      {
        if(e >= 0)
        {
          x += incx;
          e += inc1;
        }
        else e += inc2;
        y += incy;
        if(draw(app,body,x,y)) return 1;
      }
  }

  return 0;
}


int main(int argc, char* args[] )
{
  srand(time(NULL));
  App app;
  app.debug = 0;
  memset(&app, 0, sizeof(app));

  app.state = STATE_MENU;
  app.menu.selected = MENU_NEW_GAME;
  app.credits = CREDITS_TEAM;

  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 ) return 1;
  init_font();
  InitializePathfinder();
  renderInit(&app);
  soundInit();
  loadItems(&app);
  loadEnemies(&app);
  loadMap(&app, 0); // calls moveInit

  while(app.state != STATE_EXIT){
	Uint32 startTime = SDL_GetTicks();
	movePrepare(&app);
	if (app.state == STATE_PLAYING) {
	  renderStart(&app);
	}
	bindKeyboard(&app);

	if (app.state == STATE_PLAYING){
	  playRandomMusic();
	  Uint32 elapsed = startTime - app.game.spawnTime;
	  if(elapsed > 500)
	  {
      spawnEnemy(&app);
      app.game.spawnTime = startTime;
	  }
	  move_enemies(&app);
	  renderFinish(&app);
	  checkGameover(&app);
	} else if (app.state == STATE_CREDITS) {
	  renderCredits(&app);
	} else {
	  renderMenu(&app);
	  playMusic("menu.mp3", -1);
	}
	handleDelay(startTime);
  }

  return 0;
}


