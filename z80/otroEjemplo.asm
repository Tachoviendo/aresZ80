; Programa SID-Ulator para Z80 - Toca secuencia de notas
; Basado en el ejemplo BASIC del RC2014

REG     EQU $D4         ; Puerto de registro
DAT     EQU $D5         ; Puerto de datos

        ORG $8000

START:
        ; Limpiar registros SID (línea 10)
        LD B, 25        ; 0 a 24 = 25 registros
        LD C, 0         ; Empezar en registro 0
CLEAR:
        LD A, C
        OUT (REG), A
        XOR A
        OUT (DAT), A
        INC C
        DJNZ CLEAR

        ; Configurar Attack/Decay (línea 20)
        LD A, 5
        OUT (REG), A
        LD A, 9
        OUT (DAT), A
        
        LD A, 6
        OUT (REG), A
        XOR A
        OUT (DAT), A

        ; Volumen al máximo (línea 30)
        LD A, 24
        OUT (REG), A
        LD A, 15
        OUT (DAT), A

        ; Apuntar a los datos de notas
        LD IX, NOTES

PLAY_LOOP:
        ; Leer HF, LF, DR (línea 40)
        LD A, (IX+0)    ; HF (frecuencia alta)
        CP $FF          ; Verificar fin de datos (-1)
        JP Z, END_PROG
        
        LD H, A         ; Guardar HF
        LD L, (IX+1)    ; LF (frecuencia baja)
        LD E, (IX+2)    ; DR (duración)

        ; Configurar frecuencia (línea 60)
        LD A, 1         ; Registro 1 (freq alta)
        OUT (REG), A
        LD A, H         ; HF
        OUT (DAT), A
        
        LD A, 0         ; Registro 0 (freq baja)
        OUT (REG), A
        LD A, L         ; LF
        OUT (DAT), A

        ; Gate ON (línea 70)
        LD A, 4
        OUT (REG), A
        LD A, 33        ; Sawtooth + Gate
        OUT (DAT), A

        ; Delay (línea 80)
        LD D, E         ; Copiar duración
DELAY1:
        LD BC, 200      ; Delay interno ajustado
DELAY2:
        DEC BC
        LD A, B
        OR C
        JP NZ, DELAY2
        DEC D
        JP NZ, DELAY1

        ; Gate OFF (línea 90)
        LD A, 4
        OUT (REG), A
        LD A, 32        ; Sawtooth, sin Gate
        OUT (DAT), A

        ; Siguiente nota
        LD BC, 3
        ADD IX, BC
        JP PLAY_LOOP

END_PROG:
        RET

; Datos de notas (líneas 110-190)
NOTES:
        DB 25,177,250   ; Línea 110
        DB 28,214,250
        DB 25,177,250
        DB 25,177,250   ; Línea 120
        DB 25,177,250
        DB 25,177,125   ; Línea 130
        DB 28,214,125
        DB 32,94,750    ; Línea 140
        DB 25,177,250
        DB 28,214,250   ; Línea 150
        DB 19,63,250
        DB 19,63,250    ; Línea 160
        DB 19,63,250
        DB 21,154,63    ; Línea 170
        DB 24,63,63
        DB 25,177,250   ; Línea 180
        DB 24,63,125
        DB 19,63,250    ; Línea 190
        DB $FF,$FF,$FF  ; Marca de fin

        END
