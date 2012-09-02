#import "sound.h"

Mix_Music *music = NULL;
Audio a;
char* lastMusic = "";

void playMusic(char* name){
	if(lastMusic == name){
		return;
	}

	lastMusic = name;
	music = Mix_LoadMUS(name);
	if(Mix_PlayMusic(music, -1) == -1) {
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
		// well, there's no music, but most games don't break without music...
	}
}

void soundInit() {
    a.rate = 22050;
    a.format = AUDIO_S16; /* 16-bit stereo */
    a.channels = 2;
    a.buffers = 4046;

    if(Mix_OpenAudio(a.rate, a.format, a.channels, a.buffers)) {
        printf("Unable to open audio!\n");
        exit(1);
    }

    Mix_QuerySpec(&a.rate, &a.format, &a.channels);
}

void halt_music(){
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    music = NULL;
}
