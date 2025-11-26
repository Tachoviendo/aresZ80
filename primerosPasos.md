# 🎵 WAV2SID - Guía de Instalación Paso a Paso

## 📋 Estructura del Proyecto

Cuando descargues el proyecto, vas a tener esta estructura:

```
wav2sid-converter/
├── README.md              ← Documentación completa
├── INSTALL.md             ← Esta guía
├── STRUCTURE.md           ← Explicación de la estructura
├── Makefile               ← Para compilar fácilmente
├── test_wav2sid.sh        ← Script de prueba
├── .gitignore
│
├── src/
│   ├── wav2sid.c          ← Versión Linux
│   └── wav2sid_cpm.c      ← Versión CP/M
│
├── bin/                   ← Se crea al compilar
│   ├── wav2sid            (ejecutable Linux)
│   └── wav2sid.com        (ejecutable CP/M)
│
├── examples/              ← Para tus pruebas
│   └── (archivos de ejemplo)
│
└── docs/
    └── EXAMPLES.md        ← Ejemplos de uso
```

---

## 🚀 Instalación

### Paso 1: Descargar el Proyecto

Si usás git:
```bash
git clone <URL_DEL_REPO>
cd wav2sid-converter
```

O simplemente descargá y descomprimí los archivos en una carpeta llamada `wav2sid-converter/`

### Paso 2: Instalar Dependencias

**En Arch Linux:**
```bash
sudo pacman -S gcc make python ffmpeg z88dk
```

**En Ubuntu/Debian:**
```bash
sudo apt install gcc make python3 ffmpeg build-essential
# z88dk hay que instalarlo manualmente desde:
# https://github.com/z88dk/z88dk
```

### Paso 3: Compilar

```bash
cd wav2sid-converter
make
```

Esto compila ambas versiones:
- `bin/wav2sid` (Linux)
- `bin/wav2sid.com` (CP/M)

### Paso 4: Test

```bash
make test
```

Esto genera un beep de prueba y lo convierte automáticamente.

---

## 📂 ¿Dónde van los Archivos?

### Archivos que VOS creás:

Poné tus archivos de audio en `examples/`:

```bash
wav2sid-converter/
└── examples/
    ├── mi_cancion.mp3     ← Tu archivo original
    ├── mi_cancion.wav     ← Después de ffmpeg
    ├── mi_cancion.asm     ← Después de wav2sid
    └── mi_cancion.com     ← Después de z80asm
```

### Ejemplo de workflow:

```bash
# 1. Convertir tu MP3 a WAV
ffmpeg -i examples/mi_cancion.mp3 \
       -ac 1 -ar 8000 -acodec pcm_u8 \
       examples/mi_cancion.wav

# 2. WAV → ASM
bin/wav2sid examples/mi_cancion.wav \
            examples/mi_cancion.asm \
            50

# 3. ASM → COM
z80asm -b examples/mi_cancion.asm \
       -o examples/mi_cancion.com

# 4. ¡Listo! Ahora copiá examples/mi_cancion.com a tu RC2014
```

---

## 🎮 Usar en RunCPM

Si querés usar la versión CP/M:

```bash
# 1. Compilar versión CP/M
make cpm

# 2. Copiar a RunCPM
make install
# O manualmente:
cp bin/wav2sid.com ~/RunCPM/A/

# 3. Copiar tu audio
cp examples/mi_cancion.wav ~/RunCPM/A/SONG.WAV

# 4. Ejecutar RunCPM
cd ~/RunCPM
./RunCPM

# 5. Dentro de CP/M:
A> WAV2SID SONG.WAV SONG.ASM 50
A> ASM SONG
A> SONG
```

---

## 📝 Comandos Útiles

### Compilar solo Linux:
```bash
make linux
```

### Compilar solo CP/M:
```bash
make cpm
```

### Limpiar binarios:
```bash
make clean
```

### Ver ayuda:
```bash
make help
```

---

## 🗂️ Organización Recomendada

### Para proyectos individuales:

```
wav2sid-converter/
└── examples/
    ├── proyecto1/
    │   ├── audio.mp3
    │   ├── audio.wav
    │   ├── audio.asm
    │   └── audio.com
    │
    └── proyecto2/
        ├── song.mp3
        ├── song.wav
        ├── song.asm
        └── song.com
```

### Para compartir con otros:

Solo necesitás compartir el archivo `.com` final. Por ejemplo:

```bash
# Crear un ZIP con tus .com
cd examples
zip mis_canciones.zip *.com

# O compartir directamente
cp proyecto1/audio.com /mnt/sd_rc2014/
```

---

## ❓ FAQ de Instalación

### "command not found: wav2sid"

Usá la ruta completa:
```bash
./bin/wav2sid archivo.wav archivo.asm 50
```

O agregalo al PATH:
```bash
export PATH=$PATH:$(pwd)/bin
```

### "z80asm not found"

Instalá z88dk:
```bash
# Arch Linux
sudo pacman -S z88dk

# Otras distros: compilar desde source
git clone https://github.com/z88dk/z88dk
cd z88dk
./build.sh
```

### "ffmpeg not found"

```bash
# Arch
sudo pacman -S ffmpeg

# Ubuntu
sudo apt install ffmpeg
```

### No tengo RunCPM

Descargalo:
```bash
git clone https://github.com/MockbaTheBorg/RunCPM.git
cd RunCPM/RunCPM
make linux
./RunCPM
```

---

## 🎯 Próximos Pasos

1. ✅ Instalar dependencias
2. ✅ Compilar con `make`
3. ✅ Hacer `make test`
4. 📖 Leer [docs/EXAMPLES.md](docs/EXAMPLES.md)
5. 🎵 ¡Convertir tu primer audio!

---

**¡Listo para empezar!** 🚀

Para más detalles, consultá:
- [README.md](README.md) - Documentación completa
- [STRUCTURE.md](STRUCTURE.md) - Estructura del proyecto
- [docs/EXAMPLES.md](docs/EXAMPLES.md) - Ejemplos prácticos
