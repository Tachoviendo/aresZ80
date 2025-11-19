; Toca una nota simple

        ORG $8000           ; Dirección de inicio

START:
        ; Primero limpiamos todos los registros
        LD L, $18           ; 24 registros
CLEAR:
        LD A, L
        OUT ($D4), A        ; Seleccionar registro
        XOR A               ; A = 0
        OUT ($D5), A        ; Escribir 0
        DEC L
        JP NZ, CLEAR

        ; Configurar Voice 1 para tocar una nota

        ; Frecuencia baja (registro 0)
        LD A, 0
        OUT ($D4), A
        LD A, $5C           ; Frecuencia baja
        OUT ($D5), A

        ; Frecuencia alta (registro 1)
        LD A, 1
        OUT ($D4), A
        LD A, $1E           ; Frecuencia alta (nota A-4)
        OUT ($D5), A

        ; Attack/Decay (registro 5)
        LD A, 5
        OUT ($D4), A
        LD A, $09           ; Attack=0, Decay=9
        OUT ($D5), A

        ; Sustain/Release (registro 6)
        LD A, 6
        OUT ($D4), A
        LD A, $00           ; Sustain=0, Release=0
        OUT ($D5), A

        ; Control register (registro 4) - activar nota
        LD A, 4
        OUT ($D4), A
        LD A, $11           ; Gate ON, forma de onda Triangle
        OUT ($D5), A

        ; Delay para que suene un rato
        LD BC, $FFFF
DELAY:
        DEC BC
        LD A, B
        OR C
        JP NZ, DELAY

        ; Apagar nota (Gate OFF)
        LD A, 4
        OUT ($D4), A
        LD A, $10           ; Gate OFF
        OUT ($D5), A

        RET                 ; Fin del programa

        END
