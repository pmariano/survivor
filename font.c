#include "font.h"

#define TTF_POINT_LIMIT 99
TTF_Font *ttf_point_cache[TTF_POINT_LIMIT];

void init_font() {
    TTF_Init();
	memset(ttf_point_cache, 0, sizeof(ttf_point_cache));
}

TTF_Font *setup_ttf(int points){
	if(points >= TTF_POINT_LIMIT) return NULL;
    TTF_Font *ttf_tmp = ttf_point_cache[points];
	if(ttf_tmp) return ttf_tmp;
    ttf_point_cache[points] = ttf_tmp = TTF_OpenFont("data/Chalkduster.ttf", points);

    int renderstyle = TTF_STYLE_NORMAL;
    int outline = 0;
    int kerning = 1;

    TTF_SetFontStyle(ttf_tmp, renderstyle);
    return ttf_tmp;
}

void text_write(SDL_Surface *screen, int x, int y, char *message, int selected){

    SDL_Color color;
    SDL_Color red = {0x00, 0XFF, 0x00};
    SDL_Color white = {0xFF, 0XFF, 0xFF};

    color = (selected) ? white : red;

    text_write_raw(screen, x, y, message, color, 72);
}

void text_write_raw(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points) {
    TTF_Font *tmp;

    tmp = setup_ttf(points);
    SDL_Rect dstrect;
    SDL_Surface *text;

    text = TTF_RenderText_Solid(tmp, message, color);

    dstrect.x = x;
    dstrect.y = y;
    dstrect.w = text->w;
    dstrect.h = text->h;

    SDL_BlitSurface(text, NULL, screen, &dstrect);

    SDL_FreeSurface(text);
}
