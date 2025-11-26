; Código generado por WAV2SID
; 40 notas de audio
; Puerto SID: $D4 (REG), $D5 (DATA)

	ORG $0100

START:
	CALL INIT_SID
	CALL PLAY_SONG
	RST 0			; Volver a CP/M

; Inicializar SID
INIT_SID:
	; Limpiar todos los registros
	LD B, 29		; 29 registros del SID
	LD C, 0			; Valor 0
	LD A, 0			; Registro inicial
CLEAR_LOOP:
	OUT ($D4), A		; Seleccionar registro
	LD A, C
	OUT ($D5), A		; Escribir 0
	LD A, B
	DEC A
	LD B, A
	OR A
	JR NZ, CLEAR_LOOP

	; Configurar Voice 1 - Triangle wave
	LD A, $04		; Registro 4 (Control)
	OUT ($D4), A
	LD A, $11		; Triangle wave + Gate ON
	OUT ($D5), A

	; Configurar ADSR
	LD A, $05		; Registro 5 (Attack/Decay)
	OUT ($D4), A
	LD A, $09		; Attack=0, Decay=9
	OUT ($D5), A

	LD A, $06		; Registro 6 (Sustain/Release)
	OUT ($D4), A
	LD A, $F0		; Sustain=15, Release=0
	OUT ($D5), A

	RET

; Reproducir canción
PLAY_SONG:
	LD HL, NOTE_DATA
	LD B, 40		; Número de notas
PLAY_LOOP:
	PUSH BC

	; Leer frecuencia LO
	LD A, $00		; Registro 0 (Freq LO)
	OUT ($D4), A
	LD A, (HL)
	OUT ($D5), A
	INC HL

	; Leer frecuencia HI
	LD A, $01		; Registro 1 (Freq HI)
	OUT ($D4), A
	LD A, (HL)
	OUT ($D5), A
	INC HL

	; Leer volumen
	LD A, $18		; Registro 24 (Volumen global)
	OUT ($D4), A
	LD A, (HL)
	OUT ($D5), A
	INC HL

	; Delay
	LD A, (HL)
	INC HL
	CALL DELAY_MS

	POP BC
	DJNZ PLAY_LOOP
	RET

; Delay en milisegundos (A = ms)
DELAY_MS:
	OR A
	RET Z
	PUSH BC
	LD B, A
DELAY_OUTER:
	PUSH BC
	LD BC, 400		; ~1ms a 4MHz
DELAY_INNER:
	DEC BC
	LD A, B
	OR C
	JR NZ, DELAY_INNER
	POP BC
	DJNZ DELAY_OUTER
	POP BC
	RET

; Tabla de notas: freq_lo, freq_hi, volume, duration
NOTE_DATA:
	DB $2E, $1C, $09, 50	; #1: F=7214 V=9
	DB $D5, $1C, $09, 50	; #2: F=7381 V=9
	DB $D5, $1C, $09, 50	; #3: F=7381 V=9
	DB $D5, $1C, $09, 50	; #4: F=7381 V=9
	DB $D5, $1C, $09, 50	; #5: F=7381 V=9
	DB $D5, $1C, $09, 50	; #6: F=7381 V=9
	DB $D5, $1C, $09, 50	; #7: F=7381 V=9
	DB $D5, $1C, $09, 50	; #8: F=7381 V=9
	DB $D5, $1C, $09, 50	; #9: F=7381 V=9
	DB $D5, $1C, $09, 50	; #10: F=7381 V=9
	DB $D5, $1C, $09, 50	; #11: F=7381 V=9
	DB $D5, $1C, $09, 50	; #12: F=7381 V=9
	DB $D5, $1C, $09, 50	; #13: F=7381 V=9
	DB $D5, $1C, $09, 50	; #14: F=7381 V=9
	DB $D5, $1C, $09, 50	; #15: F=7381 V=9
	DB $D5, $1C, $09, 50	; #16: F=7381 V=9
	DB $D5, $1C, $09, 50	; #17: F=7381 V=9
	DB $D5, $1C, $09, 50	; #18: F=7381 V=9
	DB $D5, $1C, $09, 50	; #19: F=7381 V=9
	DB $D5, $1C, $09, 50	; #20: F=7381 V=9
	DB $D5, $1C, $09, 50	; #21: F=7381 V=9
	DB $D5, $1C, $09, 50	; #22: F=7381 V=9
	DB $D5, $1C, $09, 50	; #23: F=7381 V=9
	DB $D5, $1C, $09, 50	; #24: F=7381 V=9
	DB $D5, $1C, $09, 50	; #25: F=7381 V=9
	DB $D5, $1C, $09, 50	; #26: F=7381 V=9
	DB $D5, $1C, $09, 50	; #27: F=7381 V=9
	DB $D5, $1C, $09, 50	; #28: F=7381 V=9
	DB $D5, $1C, $09, 50	; #29: F=7381 V=9
	DB $D5, $1C, $09, 50	; #30: F=7381 V=9
	DB $D5, $1C, $09, 50	; #31: F=7381 V=9
	DB $D5, $1C, $09, 50	; #32: F=7381 V=9
	DB $D5, $1C, $09, 50	; #33: F=7381 V=9
	DB $D5, $1C, $09, 50	; #34: F=7381 V=9
	DB $D5, $1C, $09, 50	; #35: F=7381 V=9
	DB $D5, $1C, $09, 50	; #36: F=7381 V=9
	DB $D5, $1C, $09, 50	; #37: F=7381 V=9
	DB $D5, $1C, $09, 50	; #38: F=7381 V=9
	DB $D5, $1C, $09, 50	; #39: F=7381 V=9
	DB $D5, $1C, $09, 50	; #40: F=7381 V=9

