##!/bin/bash
# test_wav2sid.sh - Script de prueba rápida del conversor

set -e

echo "=== WAV2SID - Test Rápido ==="
echo ""

# Verificar dependencias
echo "[1/6] Verificando dependencias..."
command -v gcc >/dev/null 2>&1 || { echo "ERROR: gcc no instalado"; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo "ERROR: python3 no instalado"; exit 1; }
command -v z80asm >/dev/null 2>&1 || { echo "ADVERTENCIA: z80asm no instalado (necesario para ensamblar)"; }

echo "✓ Dependencias OK"
echo ""

# Compilar versión Linux
echo "[2/6] Compilando wav2sid para Linux..."
gcc -O2 -Wall wav2sid.c -o wav2sid -lm
echo "✓ wav2sid compilado"
echo ""

# Generar audio de prueba
echo "[3/6] Generando beep.wav de prueba (440 Hz, 2 segundos)..."
python3 << 'EOF'
import wave
import struct
import math

with wave.open('beep.wav', 'wb') as w:
    w.setnchannels(1)
    w.setsampwidth(1)
    w.setframerate(8000)
    
    # 2 segundos de 440 Hz (La musical)
    for i in range(16000):
        val = int(128 + 127 * math.sin(2 * math.pi * 440 * i / 8000))
        w.writeframes(struct.pack('B', val))

print("✓ beep.wav creado")
EOF
echo ""

# Convertir WAV a ASM
echo "[4/6] Convirtiendo beep.wav a beep.asm..."
./wav2sid beep.wav beep.asm 50
echo ""

# Ensamblar (si z80asm está disponible)
if command -v z80asm >/dev/null 2>&1; then
    echo "[5/6] Ensamblando beep.asm..."
    z80asm -b beep.asm -o beep.com
    echo "✓ beep.com generado"
    echo ""
    
    # Mostrar información del archivo
    echo "[6/6] Información del resultado:"
    ls -lh beep.com
    echo ""
    echo "=== ¡Listo! ==="
    echo ""
    echo "Archivo generado: beep.com"
    echo ""
    echo "Para probarlo en RunCPM:"
    echo "  1. cp beep.com ~/RunCPM/A/"
    echo "  2. cd ~/RunCPM && ./RunCPM"
    echo "  3. Dentro de CP/M: A> BEEP"
    echo ""
    echo "Para usarlo en RC2014:"
    echo "  - Copia beep.com a tu tarjeta SD"
    echo "  - En CP/M ejecuta: A> BEEP"
else
    echo "[5/6] SALTADO: z80asm no disponible"
    echo "[6/6] Archivo ASM generado: beep.asm"
    echo ""
    echo "=== Casi listo! ==="
    echo ""
    echo "Para ensamblar, instala z88dk o z80asm:"
    echo "  sudo pacman -S z88dk"
    echo ""
    echo "Luego ejecuta:"
    echo "  z80asm -b beep.asm -o beep.com"
fi

echo ""
echo "Archivos generados:"
ls -lh beep.* 2>/dev/null | grep -v "\.sh"!/bin/bash
# test_wav2sid.sh - Script de prueba rápida del conversor

set -e

echo "=== WAV2SID - Test Rápido ==="
echo ""

# Verificar dependencias
echo "[1/6] Verificando dependencias..."
command -v gcc >/dev/null 2>&1 || { echo "ERROR: gcc no instalado"; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo "ERROR: python3 no instalado"; exit 1; }
command -v z80asm >/dev/null 2>&1 || { echo "ADVERTENCIA: z80asm no instalado (necesario para ensamblar)"; }

echo "✓ Dependencias OK"
echo ""

# Compilar versión Linux
echo "[2/6] Compilando wav2sid para Linux..."
gcc -O2 -Wall wav2sid.c -o wav2sid -lm
echo "✓ wav2sid compilado"
echo ""

# Generar audio de prueba
echo "[3/6] Generando beep.wav de prueba (440 Hz, 2 segundos)..."
python3 << 'EOF'
import wave
import struct
import math

with wave.open('beep.wav', 'wb') as w:
    w.setnchannels(1)
    w.setsampwidth(1)
    w.setframerate(8000)
    
    # 2 segundos de 440 Hz (La musical)
    for i in range(16000):
        val = int(128 + 127 * math.sin(2 * math.pi * 440 * i / 8000))
        w.writeframes(struct.pack('B', val))

print("✓ beep.wav creado")
EOF
echo ""

# Convertir WAV a ASM
echo "[4/6] Convirtiendo beep.wav a beep.asm..."
./wav2sid beep.wav beep.asm 50
echo ""

# Ensamblar (si z80asm está disponible)
if command -v z80asm >/dev/null 2>&1; then
    echo "[5/6] Ensamblando beep.asm..."
    z80asm -b beep.asm -o beep.com
    echo "✓ beep.com generado"
    echo ""
    
    # Mostrar información del archivo
    echo "[6/6] Información del resultado:"
    ls -lh beep.com
    echo ""
    echo "=== ¡Listo! ==="
    echo ""
    echo "Archivo generado: beep.com"
    echo ""
    echo "Para probarlo en RunCPM:"
    echo "  1. cp beep.com ~/RunCPM/A/"
    echo "  2. cd ~/RunCPM && ./RunCPM"
    echo "  3. Dentro de CP/M: A> BEEP"
    echo ""
    echo "Para usarlo en RC2014:"
    echo "  - Copia beep.com a tu tarjeta SD"
    echo "  - En CP/M ejecuta: A> BEEP"
else
    echo "[5/6] SALTADO: z80asm no disponible"
    echo "[6/6] Archivo ASM generado: beep.asm"
    echo ""
    echo "=== Casi listo! ==="
    echo ""
    echo "Para ensamblar, instala z88dk o z80asm:"
    echo "  sudo pacman -S z88dk"
    echo ""
    echo "Luego ejecuta:"
    echo "  z80asm -b beep.asm -o beep.com"
fi

echo ""
echo "Archivos generados:"
ls -lh beep.* 2>/dev/null | grep -v "\.sh"
