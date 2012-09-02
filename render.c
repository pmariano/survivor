#include <SDL_image.h>
#include "render.h"
#include "font.h"
#include "aStarLibrary.h"

SDL_Color red = {0xAA, 0X55, 0x00};
SDL_Color trueRed = {0XFF, 0x00, 0x00};
SDL_Color white = {0xFF, 0XFF, 0xFF};
SDL_Color green = {0x00, 0XFF, 0x00};

void renderStats(SDL_Surface *screen, Player *player1, Player *player2){
  if(player1->state == PLAYER_READY){
    text_write_raw(screen, 5, 30, "Player 1", green, 30);
	Uint32 color = SDL_MapRGB(screen->format, 99, 0,0 );

	SDL_Rect rect = { 5, 10, player1->body.life * 2, 20};
	SDL_FillRect(screen, &rect, color);
  }

  if(player2->state == PLAYER_IDLE){
    text_write_raw(screen, 900, 30, "Player 2", green, 30);
	Uint32 color = SDL_MapRGB(screen->format, 99, 0,0 );

	SDL_Rect rect = { 830, 10, player2->body.life * 2, 20};
	SDL_FillRect(screen, &rect, color);
  }
}

void renderPlayer(Game *game, Player *player){
	if(player->state != PLAYER_READY) return;
	int a = player->body.angle;
	SDL_Surface *image;
	if(a >= 315 || a < 45) image = player->right;
	else if(a >= 45 && a < 135) image = player->up;
	else if(a >= 135 && a < 225) image = player->left;
	else if(a >= 225 && a < 315) image = player->down;
	else return;
	SDL_Rect rect = {
		player->body.pos.x - image->w/2,
		player->body.pos.y - image->h*3/4,
		player->body.pos.w,
		player->body.pos.h
	};
	int j = game->board.sprite_count++;
	game->board.sprite[j].image = image;
	game->board.sprite[j].rect = rect;

	rect.y++;
	rect.x+=image->w/2;
	j = game->board.sprite_count++;
	game->board.sprite[j].image = player->body.item.type->image;
	printf("item %p\n", player->body.item.type);
	game->board.sprite[j].rect = rect;
}

void renderEnemies(App *app)
{
  int i = 0;
  for(; i < ENEMY_COUNT; i++)
  {
    if(app->game.enemies[i].state == ENEMY_LIVE)
    {
      Enemy *enemy = &app->game.enemies[i];
      SDL_Surface *image = enemy->image;
      SDL_Rect rect = {
        enemy->body.pos.x - image->w/2,
        enemy->body.pos.y - image->h*3/4,
        enemy->body.pos.w,
        enemy->body.pos.h
      };
		int j = app->game.board.sprite_count++;
		app->game.board.sprite[j].image = image;
		app->game.board.sprite[j].rect = rect;
    }
  }
}

int sprite_zsort(const void *a, const void *b)
{
	Sprite *aa = (Sprite *)a;
	Sprite *bb = (Sprite *)b;
	return aa->rect.y - bb->rect.y;
}

void flushRender(App *app)
{
	int i;
	qsort(app->game.board.sprite, app->game.board.sprite_count, sizeof(Sprite), sprite_zsort);

	for(i=0; i< app->game.board.sprite_count; i++) {
		SDL_BlitSurface(app->game.board.sprite[i].image, NULL, app->screen, &app->game.board.sprite[i].rect);
	}
}

void renderPowerups(App *app)
{
  int i = 0;
  for(; i < POWERUP_COUNT; i++)
  {
    if(app->game.board.powerups[i].should_show == 1)
    {
      SDL_Rect rect = {
        app->game.board.powerups[i].x,
        app->game.board.powerups[i].y,
        app->game.board.powerups[i].type->image->w,
        app->game.board.powerups[i].type->image->h
      };
      int j = app->game.board.sprite_count++;
      app->game.board.sprite[j].image = app->game.board.powerups[i].type->image;
      app->game.board.sprite[j].rect = rect;
    }
  }

}

void addPowerup(App *app)
{
	int i;
	int x,y;
	if(powerup_spawn_pos(&app->game, &x, &y)) {
		for(i=0; i < POWERUP_COUNT; i++) {
			if(app->game.board.powerups[i].should_show == 0) { 
				app->game.board.powerups[i].should_show = 1;
				app->game.board.powerups[i].ammo_used = 0;
				app->game.board.powerups[i].type = &app->game.itemtype[rand()%ITEM_PLAYER_COUNT];
				app->game.board.powerups[i].x = x;
				app->game.board.powerups[i].y = y;
				app->game.board.powerup[x/tileSize][y/tileSize] = 1+i;
				break;
			}
		}
	}
}

void renderStart(App *app){
  int x,y;
  app->game.board.sprite_count = 0;

  Game game = app->game;

  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_FillRect(app->screen, NULL , color);

  SDL_BlitSurface(app->game.board.image, NULL, app->screen, NULL);

  if(app->debug){
    for (x=0; x < mapWidth;x++) {
      for (y=0; y < mapHeight;y++) {
        //if(walkability[x][y]) {
        // if(app->game.board.wall[x][y]) {
        //if(app->game.board.crowd[x][y]) {
        if(app->game.board.powerup[x][y]) {
          SDL_Rect rect = { x*tileSize, y*tileSize, tileSize, tileSize };
          SDL_FillRect(app->screen, &rect , 0xffffff);
        }
      }
    }
  }

}

void renderFinish(App *app){
  Game game = app->game;

  renderPlayer(&app->game, &game.player1);
  renderPlayer(&app->game, &game.player2);
  renderEnemies(app);
  if((rand() % 30) == 0)
  {
    addPowerup(app);
  }
  renderPowerups(app);
  //SDL_UpdateRect(app->screen, 0, 0, 0, 0);

  flushRender(app);
  renderStats(app->screen, &game.player1, &game.player2);

  SDL_Flip(app->screen);
}


void renderInit(App *app){
  app->menu.soldier = IMG_Load("data/soldado1_grande.png");
  app->menu.bigZombie = IMG_Load("data/zombie2_grande.png");
  app->menu.zombie = IMG_Load("data/zombie1.png");
  app->menu.engineer = IMG_Load("data/engenheiro1.png");
  app->game.player1.up = IMG_Load("data/soldado1_costas.png");
  app->game.player1.down = IMG_Load("data/soldado1.png");
  app->game.player1.left = IMG_Load("data/soldado1.png");
  app->game.player1.right = IMG_Load("data/soldado1_costas.png");
  app->game.player2.up = IMG_Load("data/engenheiro1.png");
  app->game.player2.down = IMG_Load("data/engenheiro1.png");
  app->game.player2.left = IMG_Load("data/engenheiro1.png");
  app->game.player2.right = IMG_Load("data/engenheiro1.png");
  app->game.enemy_class[ENEMY_MEDIC].image = IMG_Load("data/zombie1.png");
  app->game.enemy_class[ENEMY_SOLDIER].image = IMG_Load("data/zombie2.png");

  app->screen = SDL_SetVideoMode(1024, 768, 32, SDL_HWSURFACE);
}

/**
 *
 * TODO: refactor this to be part of render func
 */
void renderMenu(App *app){
	Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
	SDL_FillRect(app->screen, NULL , color);
	Menu *menu = &app->menu;
	SDL_Surface *screen = app->screen;

	SDL_Rect titleCharPos = {-700, -50, screen->w, screen->h};
	SDL_BlitSurface(menu->zombie, &titleCharPos, screen, NULL);

	if(app->state == STATE_GAMEOVER) {
	  SDL_Rect highlightChar = {-750, -450, screen->w, screen->h};
	  SDL_BlitSurface(menu->bigZombie, &highlightChar, screen, NULL);
	} else {
	  SDL_Rect highlightChar = {-700, -450, screen->w, screen->h};
	  SDL_BlitSurface(menu->soldier, &highlightChar, screen, NULL);
	}

	int resumePadding = 0;

	if(app->state == STATE_GAMEOVER){
	  resumePadding = 100;

	  char kills[256];
	  sprintf(kills, "You died. Zombies killed: %i", app->game.kill_count);
	  text_write_raw(screen, 100, 250, kills, trueRed, 50);
	} else if(app->state == STATE_PAUSED){
	  resumePadding = 100;
	  text_write(screen, 100, 250, "resume game", menu->selected == MENU_RESUME);
	}

    text_write_raw(screen, 300, 50, "Survivor", red, 96);

	text_write(screen, 100, 250 + resumePadding, "new game", menu->selected == MENU_NEW_GAME);
	text_write(screen, 100, 450 + resumePadding, "credits", menu->selected == MENU_CREDITS);
	text_write(screen, 100, 550 + resumePadding, "exit", menu->selected == MENU_QUIT);

  SDL_UpdateRect(app->screen, 0, 0, 0, 0);
}

void renderCredits(App *app)
{
  Uint32 color = SDL_MapRGB(app->screen->format, 33, 33,33 );
  SDL_Surface *screen = app->screen;
  SDL_FillRect(screen, NULL , color);

  SDL_Rect charPos = {-670, -50, screen->w, screen->h};
  SDL_BlitSurface(app->menu.engineer, &charPos, screen, NULL );

  text_write_raw(screen, 300, 50, "Credits", red, 96);

  if(app->credits != CREDITS_SOUND){
	text_write_raw(screen, 100, 150, "team", green, 36);
	text_write_raw(screen, 100, 200, "Carlo \"zED\" Caputo", white, 26);
	text_write_raw(screen, 100, 250, "Pedro Mariano", white, 26);
	text_write_raw(screen, 100, 300, "Caires Vinicius", white, 26);

	text_write_raw(screen, 100, 400, "chars", green, 36);
	text_write_raw(screen, 100, 450, "based on http://pixelblock.tumblr.com", white, 26);
	text_write_raw(screen, 100, 500, "modified by Pedro Jatoba", white, 26);

	text_write_raw(screen, 100, 600, "tileset", green, 36);
	text_write_raw(screen, 100, 650, "lost garden", white, 26);
	text_write_raw(screen, 400, 600, "font", green, 36);
	text_write_raw(screen, 400, 650, "Pixelsix, 2005 by Cal Henderson", white, 26);
  } else {
	text_write_raw(screen, 100, 150, "Musics", green, 36);
	text_write_raw(screen, 100, 200, "cluck, Computer Savvy - The J Arthut Keenes band", white, 26);
	text_write_raw(screen, 100, 250, "Come and Find Us Remix, Resistor Anthems - Eric Skiff", white, 26);
	text_write_raw(screen, 100, 300, "Arpanauts, Resistor Anthems - Eric Skiff", white, 26);
	text_write_raw(screen, 100, 350, "Underclocked (mix), Resistor Anthems - Eric Skiff", white, 26);

	text_write_raw(screen, 100, 450, "Sounds", green, 36);

	text_write_raw(screen, 100, 500, "http://www.freesound.org/people/LAGtheNoggin/sounds/15545/", white, 26);
	text_write_raw(screen, 100, 550, "http://www.freesound.org/people/Sparrer/sounds/50506/", white, 26);
	text_write_raw(screen, 100, 600, "http://www.freesound.org/people/DJ20Chronos/sounds/33380/", white, 26);
  }

  SDL_UpdateRect(screen, 0, 0, 0, 0);
}



