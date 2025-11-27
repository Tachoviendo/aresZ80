# Arez80 - Conversor de Audio para SID-Ulator RC2014

Conversor de archivos de audio WAV a código ensamblador Z80 para reproducción en el módulo SID-Ulator del RC2014.

##  Descripción

**Arez80** es una herramienta desarrollada como parte del TBA 2  del curso de Microprocesadores en la Universidad Católica del Uruguay, Campus Salto. El proyecto permite convertir archivos de audio en código ensamblador ejecutable por el microprocesador Z80, diseñado específicamente para controlar el módulo SID-Ulator que emula el chip de sonido SID (Sound Interface Device) de Commodore 64.

##  Objetivo

Implementar un reproductor de música controlado por el microprocesador Z80, convirtiendo archivos de audio en instrucciones que escriben directamente a los registros del SID-Ulator a través de los puertos D4H (selección de registro) y D5H (escritura de dato).

##  Características

- ✅ Conversión de archivos WAV (8-bit mono, 8000 Hz) a código ensamblador Z80
- ✅ Detección de frecuencias mediante análisis de cruces por cero
- ✅ Generación automática de código ASM con inicialización del SID
- ✅ Configuración de envelope ADSR para Voice 1
- ✅ Delays precisos entre notas
- ✅ Compatible con CP/M y ejecutable en RunCPM
- ✅ Ensamblado automático a formato .COM

##  Arquitectura

El proyecto se basa en el análisis del esquema del **SID-Ulator v1.1**, que incluye:

- **ATmega328p**: Emula el chip SID original
- **74HCT138**: Decodificador de direcciones (3 a 8 líneas)
- **74AHCT374**: Registro latch octal para estabilizar datos
- **74LS688**: Comparador de magnitud para selección de direcciones
- **74HC86**: Lógica de control mediante compuertas XOR
- **Bus RC2014**: Conexión con el Z80 (direcciones A0-A15, datos D0-D7, señales de control)

##  Instalación

### Prerequisitos

**En Arch Linux:**
```bash
sudo pacman -S gcc make python ffmpeg z88dk
```

**En Ubuntu/Debian:**
```bash
sudo apt install gcc make python3 ffmpeg build-essential
# z88dk: https://github.com/z88dk/z88dk
```

### Compilar el proyecto

```bash
# Clonar el repositorio
git clone https://github.com/Tachoviendo/aresZ80.git
cd aresZ80

# Compilar versión Linux
make linux

# (Opcional) Compilar versión CP/M
make cpm
```

##  Uso

### 1. Preparar el archivo de audio

El audio debe ser **WAV de 8 bits mono a 8000 Hz**:

```bash
# Convertir MP3 a WAV compatible
ffmpeg -i cancion.mp3 -ac 1 -ar 8000 -acodec pcm_u8 -t 15 cancion.wav

# Desde cualquier formato
ffmpeg -i audio.* -ac 1 -ar 8000 -acodec pcm_u8 salida.wav
```

### 2. Convertir WAV a ASM

```bash
# Sintaxis básica
./bin/wav2sid <entrada.wav> <salida.asm> [duracion_frame_ms]

# Ejemplo (50ms por frame)
./bin/wav2sid ejemplos/cancion.wav ejemplos/cancion.asm 50
```

**Parámetro `duracion_frame_ms`:**
- `20-30ms`: Más detalle, archivo más grande
- `50ms`: Balance (por defecto) ⭐
- `80-100ms`: Menos detalle, archivo más pequeño

### 3. Ensamblar a .COM

```bash
cd ejemplos
z80asm -b cancion.asm
mv cancion.bin cancion.com
```

### 4. Ejecutar

#### En RunCPM (emulador):

```bash
# Copiar a RunCPM
cp cancion.com ~/RunCPM/A/0/CANCION.COM

# Ejecutar RunCPM
cd ~/RunCPM
./RunCPM

# Dentro de CP/M:
A0> CANCION
```

**Nota**: En RunCPM no se escucha audio real porque falta el hardware del SID-Ulator.

#### En RC2014 real:

1. Copiar `cancion.com` a la tarjeta SD del RC2014
2. Conectar el módulo SID-Ulator al bus RC2014
3. Ejecutar en CP/M: `A> CANCION`
4. ¡Escuchar el audio! 🎵

##  Prueba rápida

```bash
# Generar beep de prueba y convertirlo
make test

# Resultado: ejemplos/beep.com
```

## 📁 Estructura del Proyecto

```
aresZ80/
├── src/
│   ├── wav2sid.c          # Conversor para Linux
│   └── wav2sid_cpm.c      # Conversor para CP/M
├── bin/
│   ├── wav2sid            # Ejecutable Linux
│   └── wav2sid.com        # Ejecutable CP/M
├── ejemplos/
│   └── beep.*             # Archivos de prueba
├── docs/
│   └── TBA_2.pdf          # Documentación completa del proyecto
├── Makefile
└── README.md
```
**Equipo:**
- Ignacio Silva
- Renzo Beretta  
- Lucas Chiappini

**Tutor:** Jonatan Piuma

**Fecha:** Noviembre 2025

##  Referencias

- [RC2014 Official Website](https://rc2014.co.uk/)
- [SID-Ulator Sound Module](https://rc2014.co.uk/modules/sid-ulator-sound-module/)
- [RunCPM](https://github.com/MockbaTheBorg/RunCPM)
- [z88dk](https://github.com/z88dk/z88dk)
- Zaks, R. (1983). *Programación del Z80*. Anaya Editorial.

##  Licencia

Este proyecto es de código abierto y está disponible para cualquier fin.


