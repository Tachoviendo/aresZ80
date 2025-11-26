# Instalación Rápida

## Prerequisitos

### Arch Linux
```bash
sudo pacman -S gcc make python z88dk
```

### Ubuntu/Debian
```bash
sudo apt install gcc make python3 build-essential
# z88dk: https://github.com/z88dk/z88dk/wiki/installation
```

### Fedora
```bash
sudo dnf install gcc make python3
```

## Instalación

```bash
# 1. Clonar el repo
git clone <tu-repo-url>
cd wav2sid-converter

# 2. Compilar
make

# 3. Test rápido
./test_wav2sid.sh
```

## Uso Básico

```bash
# Convertir audio
ffmpeg -i song.mp3 -ac 1 -ar 8000 -acodec pcm_u8 song.wav
./bin/wav2sid song.wav song.asm 50

# Ensamblar
z80asm -b song.asm -o song.com

# ¡Listo!
```

Ver [README.md](README.md) para documentación completa.
