#include "font.h"

extern unsigned char adler_ttf[];
extern unsigned int adler_ttf_len;

void init_font() {
    TTF_Init();
}

TTF_Font *setup_ttf_adler(int points){
    TTF_Font *ttf_tmp = TTF_OpenFontRW(SDL_RWFromMem(adler_ttf, adler_ttf_len), 1, points);

    int renderstyle = TTF_STYLE_NORMAL;
    int outline = 0;
    int kerning = 1;

    TTF_SetFontStyle(ttf_tmp, renderstyle);
    return ttf_tmp;
}

void text_write(SDL_Surface *screen, int x, int y, char *message, int selected){

    SDL_Color color;
    SDL_Color red = {0xFF, 0X00, 0x00};
    SDL_Color white = {0xFF, 0XFF, 0xFF};

    color = (selected) ? white : red;

    text_write_raw(screen, x, y, message, color, 72);
}

void text_write_raw(SDL_Surface *screen, int x, int y, char *message, SDL_Color color, int points) {
    TTF_Font *tmp;

    tmp = setup_ttf_adler(points);
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
