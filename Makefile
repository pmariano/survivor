ifdef WIN
  CFLAGS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --cflags` -I/opt/SDL-1.2.13/include/ -I/opt/SDL-1.2.13/include/SDL/
  LIBS=`/opt/SDL-1.2.13/bin/i586-mingw32msvc-sdl-config --libs` -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf
  CC=i586-mingw32msvc-gcc
  OUTPUT=survivor.exe
else
  CFLAGS=`sdl-config --cflags` -ggdb
  LIBS=`sdl-config --libs` -L/opt/local/lib -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lSDL_ttf
  CC=gcc
  OUTPUT=survivor
endif

all: $(OUTPUT)

clean:
	rm -fv survivor survivor.exe *.o

%.o : %.c
	$(CC) $(CFLAGS) $< -c -o $@

survivor.o: survivor.c
	$(CC) $(CFLAGS) $< -c -o $@

$(OUTPUT): survivor.o render.o font.o adler.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

