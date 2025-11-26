# Makefile para WAV2SID
# Compila versión Linux y CP/M

CC = gcc
ZCC = zcc
CFLAGS = -O2 -Wall
ZCFLAGS = +cpm -subtype=default -create-app

# Directorios
SRC_DIR = src
BIN_DIR = bin
EXAMPLES_DIR = ejemplos
RUNCPM_DIR = $(HOME)/RunCPM/A

# Crear directorios si no existen
$(shell mkdir -p $(BIN_DIR) $(EXAMPLES_DIR))

# Targets
all: linux

linux: $(BIN_DIR)/wav2sid

cpm: $(BIN_DIR)/wav2sid.com

# Versión Linux
$(BIN_DIR)/wav2sid: $(SRC_DIR)/wav2sid.c
	$(CC) $(CFLAGS) $(SRC_DIR)/wav2sid.c -o $(BIN_DIR)/wav2sid -lm
	@echo "✓ wav2sid (Linux) compilado en $(BIN_DIR)/"

# Versión CP/M
$(BIN_DIR)/wav2sid.com: $(SRC_DIR)/wav2sid_cpm.c
	$(ZCC) $(ZCFLAGS) $(SRC_DIR)/wav2sid_cpm.c -o wav2sid
	@mv wav2sid.com $(BIN_DIR)/ 2>/dev/null || true
	@echo "✓ wav2sid.com (CP/M) compilado en $(BIN_DIR)/"

# Instalar en RunCPM
install: cpm
	@if [ -d "$(RUNCPM_DIR)" ]; then \
		cp $(BIN_DIR)/wav2sid.com $(RUNCPM_DIR)/; \
		echo "✓ wav2sid.com copiado a RunCPM"; \
	else \
		echo "⚠ RunCPM no encontrado en $(RUNCPM_DIR)"; \
		echo "  Copia manualmente: cp $(BIN_DIR)/wav2sid.com <tu_runcpm>/A/"; \
	fi

# Limpiar
clean:
	rm -f $(BIN_DIR)/wav2sid $(BIN_DIR)/wav2sid.com $(BIN_DIR)/*.o
	rm -f $(EXAMPLES_DIR)/*.wav $(EXAMPLES_DIR)/*.asm $(EXAMPLES_DIR)/*.com $(EXAMPLES_DIR)/*.bin $(EXAMPLES_DIR)/*.o
	@echo "✓ Limpieza completa"

# Test rápido (crea beep.wav y lo convierte)
test: linux
	@echo "Generando beep.wav de prueba..."
	@python3 -c "import wave, struct, math; w = wave.open('$(EXAMPLES_DIR)/beep.wav', 'wb'); w.setnchannels(1); w.setsampwidth(1); w.setframerate(8000); [w.writeframes(struct.pack('B', int(128 + 127 * math.sin(2 * 3.14159 * 440 * i / 8000)))) for i in range(16000)]; w.close()"
	@echo "Convirtiendo con wav2sid..."
	$(BIN_DIR)/wav2sid $(EXAMPLES_DIR)/beep.wav $(EXAMPLES_DIR)/beep.asm 50
	@if command -v z80asm >/dev/null 2>&1; then \
		echo "Ensamblando con z80asm..."; \
		cd $(EXAMPLES_DIR) && z80asm -b beep.asm && mv beep.bin beep.com; \
		echo "✓ Test completado: $(EXAMPLES_DIR)/beep.com generado"; \
	else \
		echo "⚠ z80asm no disponible, solo se generó el .asm"; \
	fi

# Ayuda
help:
	@echo "WAV2SID Makefile"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  make          - Compilar versión Linux"
	@echo "  make linux    - Compilar versión Linux"
	@echo "  make test     - Test rápido con beep"
	@echo "  make clean    - Limpiar binarios"
	@echo "  make help     - Esta ayuda"
	@echo ""
	@echo "Estructura del proyecto:"
	@echo "  src/          - Código fuente"
	@echo "  bin/          - Binarios compilados"
	@echo "  ejemplos/     - Ejemplos y tests"
	@echo ""
	@echo "Uso básico:"
	@echo "  1. Convertir MP3 a WAV:"
	@echo "     ffmpeg -i song.mp3 -ac 1 -ar 8000 -acodec pcm_u8 ejemplos/song.wav"
	@echo ""
	@echo "  2. Convertir WAV a ASM:"
	@echo "     $(BIN_DIR)/wav2sid ejemplos/song.wav ejemplos/song.asm 50"
	@echo ""
	@echo "  3. Ensamblar a COM:"
	@echo "     cd ejemplos && z80asm -b song.asm && mv song.bin song.com"
	@echo ""
	@echo "  4. Copiar a RC2014 y ejecutar en CP/M"

.PHONY: all linux cpm install clean test help
