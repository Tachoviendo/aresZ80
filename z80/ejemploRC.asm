; Conversión del ejemplo C64 SID a Z80 assembler
; Toca una nota con envelope

        ORG $8000

START:
        ; Limpiar registros primero
        LD L, $18
CLEAR:
        LD A, L
        OUT ($D4), A
        XOR A
        OUT ($D5), A
        DEC L
        JP NZ, CLEAR

        ; Calcular frecuencia (equivalente a líneas 15-35 del BASIC)
        ; A=2^(1/12) ≈ 1.059463
        ; N=-9
        ; F0=7454
        ; NF=INT(F0*A^N)
        ; FH=INT(NF/256) : FL=NF-256*FH
        ; Para N=-9, frecuencia ≈ 4435
        ; FL = 83 ($53), FH = 17 ($11)

        ; POKE S,FL  -> Línea 40
        LD A, 0             ; Registro 0 (frecuencia baja)
        OUT ($D4), A
        LD A, $53           ; FL
        OUT ($D5), A

        ; WF=32  -> Línea 45 (no se usa en este ejemplo)

        ; POKE S+5,13*16+5  -> Línea 50 (Attack/Decay)
        LD A, 5             ; Registro 5
        OUT ($D4), A
        LD A, $D5           ; 13*16+5 = 208+5 = 213 = $D5
        OUT ($D5), A

        ; POKE S+6,12*16+10  -> Línea 55 (Sustain/Release)
        LD A, 6             ; Registro 6
        OUT ($D4), A
        LD A, $CA           ; 12*16+10 = 192+10 = 202 = $CA
        OUT ($D5), A

        ; POKE S+24,15  -> Línea 60 (Volumen)
        LD A, 24            ; Registro 24
        OUT ($D4), A
        LD A, 15
        OUT ($D5), A

        ; DR=2000  -> Línea 65
        LD BC, 8000         ; Aumentado para Z80 más rápido

        ; POKE S+4,WF+1  -> Línea 70 (Gate ON)
        LD A, 4             ; Registro 4 (control)
        OUT ($D4), A
        LD A, 33            ; WF+1 = 32+1 = 33 (sawtooth + gate)
        OUT ($D5), A

        ; FOR T=1 TO DR :NEXT  -> Línea 75 (delay)
DELAY:
        DEC BC
        LD A, B
        OR C
        JP NZ, DELAY

        ; POKE S+4,WF+0  -> Línea 80 (Gate OFF)
        LD A, 4
        OUT ($D4), A
        LD A, 32            ; WF+0 = 32 (sawtooth, gate off)
        OUT ($D5), A

        RET

        END
