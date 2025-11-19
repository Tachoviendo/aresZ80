
CC = gcc
CFLAGS = -Wall -O2
LIBS = -lmpg123

all: mp3_to_z80

mp3_to_z80: mp3_to_z80.c
	$(CC) $(CFLAGS) -o mp3_to_z80 mp3_to_z80.c $(LIBS)

clean:
	rm -f mp3_to_z80 *.wav *.raw *.h

install-deps:
	@echo "Instalando dependencias en Arch Linux..."
	sudo pacman -S mpg123

test: mp3_to_z80
	@echo "Ejemplo de uso:"
	@echo "./mp3_to_z80 cancion.mp3 cancion.wav"
	@echo "./mp3_to_z80 cancion.mp3 cancion.raw -raw"
	@echo "./mp3_to_z80 cancion.mp3 cancion -h -r 11025"

.PHONY: all clean install-deps test
