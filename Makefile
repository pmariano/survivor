ifdef WIN
  CFLAGS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --cflags` -I/opt/SDL-1.2.13/include/ -I/opt/SDL-1.2.13/include/SDL/
  LIBS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf 
  # CC=i586-mingw32msvc-gcc
  CC=i386-mingw32-gcc
  OUTPUT=survivor.exe
else
  CFLAGS=`sdl-config --cflags` -ggdb -pg
  LIBS=`sdl-config --libs` -L/opt/local/lib -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf -pg
  CC=gcc
  OUTPUT=survivor
endif

.PHONY: all clean depend

all: depend $(OUTPUT)

clean:
	rm -fv survivor survivor.exe *.o .depend

depend: .depend

.depend: $(wildcard *.c)
	$(CC) $(CFLAGS) -MM $^ > .depend

include .depend

$(OUTPUT): survivor.o render.o font.o movement.o aStarLibrary.o sound.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

