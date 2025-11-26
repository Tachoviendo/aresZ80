/*
 * WAV2SID_CPM.C - Versión para CP/M
 * 
 * Uso en CP/M: WAV2SID INPUT.WAV OUTPUT.ASM
 * 
 * Compilar: zcc +cpm -O3 wav2sid_cpm.c -o wav2sid -create-app
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NOTES 500  /* Reducido para CP/M (poca RAM) */
#define SID_PORT_REG  0xD4
#define SID_PORT_DATA 0xD5

/* Estructura simplificada de nota */
typedef struct {
    unsigned int freq;
    unsigned char vol;
    unsigned char duration;
} Note;

Note notes[MAX_NOTES];
int note_count = 0;

/* Estructura WAV header simplificada */
typedef struct {
    char riff[4];
    unsigned long size;
    char wave[4];
    char fmt[4];
    unsigned long fmt_size;
    unsigned int format;
    unsigned int channels;
    unsigned long sample_rate;
    unsigned long byte_rate;
    unsigned int block_align;
    unsigned int bits;
} WAVHeader;

/* Detección de frecuencia simplificada (cruces por cero) */
unsigned int detect_freq(unsigned char *buf, int len, unsigned long rate) {
    int crossings = 0;
    int prev = (buf[0] >= 128);
    int i;
    
    for (i = 1; i < len; i++) {
        int curr = (buf[i] >= 128);
        if (curr != prev) crossings++;
        prev = curr;
    }
    
    /* freq = (crossings/2) / time */
    return (unsigned int)((crossings * rate) / (len * 2));
}

/* Calcular volumen promedio */
unsigned char calc_volume(unsigned char *buf, int len) {
    long sum = 0;
    int i, val;
    
    for (i = 0; i < len; i++) {
        val = buf[i] - 128;
        if (val < 0) val = -val;
        sum += val;
    }
    
    val = (int)(sum / len);
    val = (val * 15) / 128;
    if (val > 15) val = 15;
    
    return (unsigned char)val;
}

/* Convertir Hz a registro SID */
unsigned int hz_to_sid(unsigned int hz) {
    /* freq_reg = hz * 16.777216 */
    unsigned long result = (unsigned long)hz * 17;
    if (result > 65535) result = 65535;
    return (unsigned int)result;
}

/* Parsear WAV y extraer notas */
int parse_wav(char *filename, int frame_ms) {
    FILE *fp;
    WAVHeader hdr;
    char chunk[4];
    unsigned long chunk_size;
    unsigned char *buffer;
    int frame_samples, total_frames, i;
    unsigned long rate;
    
    fp = fopen(filename, "rb");
    if (!fp) {
        printf("ERROR: No se puede abrir %s\n", filename);
        return 0;
    }
    
    /* Leer cabecera básica */
    fread(&hdr, 1, 44, fp);
    
    if (strncmp(hdr.riff, "RIFF", 4) != 0) {
        printf("ERROR: No es un WAV válido\n");
        fclose(fp);
        return 0;
    }
    
    if (hdr.channels != 1) {
        printf("ERROR: Solo mono (channels=%u)\n", hdr.channels);
        fclose(fp);
        return 0;
    }
    
    if (hdr.bits != 8) {
        printf("ERROR: Solo 8 bits (bits=%u)\n", hdr.bits);
        fclose(fp);
        return 0;
    }
    
    rate = hdr.sample_rate;
    printf("WAV: %lu Hz, ", rate);
    
    /* Buscar chunk "data" */
    while (1) {
        if (fread(chunk, 1, 4, fp) != 4) break;
        fread(&chunk_size, 1, 4, fp);
        
        if (strncmp(chunk, "data", 4) == 0) {
            break;
        }
        fseek(fp, chunk_size, SEEK_CUR);
    }
    
    printf("%lu bytes\n", chunk_size);
    
    /* Calcular frames */
    frame_samples = (int)((rate * frame_ms) / 1000);
    total_frames = (int)(chunk_size / frame_samples);
    
    if (total_frames > MAX_NOTES) {
        total_frames = MAX_NOTES;
        printf("AVISO: Limitado a %d notas\n", MAX_NOTES);
    }
    
    printf("Frames: %d (%d ms c/u)\n", total_frames, frame_ms);
    
    /* Alocar buffer para un frame */
    buffer = (unsigned char *)malloc(frame_samples);
    if (!buffer) {
        printf("ERROR: Sin memoria\n");
        fclose(fp);
        return 0;
    }
    
    /* Procesar frames */
    note_count = 0;
    for (i = 0; i < total_frames; i++) {
        if (fread(buffer, 1, frame_samples, fp) != frame_samples) {
            break;
        }
        
        /* Detectar frecuencia y volumen */
        notes[i].freq = hz_to_sid(detect_freq(buffer, frame_samples, rate));
        notes[i].vol = calc_volume(buffer, frame_samples);
        notes[i].duration = frame_ms;
        
        /* Filtrar ruido */
        if (notes[i].vol < 2) {
            notes[i].freq = 0;
            notes[i].vol = 0;
        }
        
        note_count++;
        
        /* Mostrar progreso cada 50 notas */
        if ((i % 50) == 0) {
            printf(".");
        }
    }
    
    printf("\nNotas: %d\n", note_count);
    
    free(buffer);
    fclose(fp);
    return 1;
}

/* Generar archivo ASM */
void generate_asm(char *filename) {
    FILE *fp;
    int i;
    unsigned char lo, hi;
    
    fp = fopen(filename, "w");
    if (!fp) {
        printf("ERROR: No se puede crear %s\n", filename);
        return;
    }
    
    /* Cabecera */
    fprintf(fp, "; WAV2SID para SIDulator\n");
    fprintf(fp, "; %d notas\n\n", note_count);
    fprintf(fp, "\tORG 0100h\n\n");
    fprintf(fp, "START:\n");
    fprintf(fp, "\tCALL INIT\n");
    fprintf(fp, "\tCALL PLAY\n");
    fprintf(fp, "\tRST 0\n\n");
    
    /* Init */
    fprintf(fp, "INIT:\n");
    fprintf(fp, "\tLD B, 29\n");
    fprintf(fp, "\tLD A, 0\n");
    fprintf(fp, "CLR:\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tXOR A\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tDJNZ CLR\n\n");
    
    fprintf(fp, "\tLD A, 04h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, 11h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tLD A, 05h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, 09h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tLD A, 06h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, 0F0h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tRET\n\n");
    
    /* Play */
    fprintf(fp, "PLAY:\n");
    fprintf(fp, "\tLD HL, DATA\n");
    fprintf(fp, "\tLD B, %d\n", note_count);
    fprintf(fp, "LOOP:\n");
    fprintf(fp, "\tPUSH BC\n");
    fprintf(fp, "\tLD A, 00h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tINC HL\n");
    fprintf(fp, "\tLD A, 01h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tINC HL\n");
    fprintf(fp, "\tLD A, 18h\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tOUT (%02Xh), A\n", SID_PORT_DATA);
    fprintf(fp, "\tINC HL\n");
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tINC HL\n");
    fprintf(fp, "\tCALL DELAY\n");
    fprintf(fp, "\tPOP BC\n");
    fprintf(fp, "\tDJNZ LOOP\n");
    fprintf(fp, "\tRET\n\n");
    
    /* Delay */
    fprintf(fp, "DELAY:\n");
    fprintf(fp, "\tOR A\n");
    fprintf(fp, "\tRET Z\n");
    fprintf(fp, "\tPUSH BC\n");
    fprintf(fp, "\tLD B, A\n");
    fprintf(fp, "DL1:\n");
    fprintf(fp, "\tPUSH BC\n");
    fprintf(fp, "\tLD BC, 400\n");
    fprintf(fp, "DL2:\n");
    fprintf(fp, "\tDEC BC\n");
    fprintf(fp, "\tLD A, B\n");
    fprintf(fp, "\tOR C\n");
    fprintf(fp, "\tJR NZ, DL2\n");
    fprintf(fp, "\tPOP BC\n");
    fprintf(fp, "\tDJNZ DL1\n");
    fprintf(fp, "\tPOP BC\n");
    fprintf(fp, "\tRET\n\n");
    
    /* Data */
    fprintf(fp, "DATA:\n");
    for (i = 0; i < note_count; i++) {
        lo = notes[i].freq & 0xFF;
        hi = (notes[i].freq >> 8) & 0xFF;
        fprintf(fp, "\tDB %02Xh,%02Xh,%02Xh,%d\n",
                lo, hi, notes[i].vol, notes[i].duration);
    }
    
    fprintf(fp, "\nEND START\n");
    
    fclose(fp);
    printf("ASM generado: %s\n", filename);
}

/* Convertir a mayúsculas (CP/M style) */
void to_upper(char *s) {
    while (*s) {
        *s = toupper(*s);
        s++;
    }
}

int main(int argc, char *argv[]) {
    int frame_ms = 50;
    
    printf("WAV2SID v1.0 para CP/M\n\n");
    
    if (argc < 3) {
        printf("Uso: WAV2SID INPUT.WAV OUTPUT.ASM [MS]\n");
        printf("\nEjemplo:\n");
        printf("  A> WAV2SID SONG.WAV SONG.ASM 50\n\n");
        return 1;
    }
    
    if (argc >= 4) {
        frame_ms = atoi(argv[3]);
        if (frame_ms < 10 || frame_ms > 200) {
            printf("Frame debe ser 10-200ms\n");
            return 1;
        }
    }
    
    /* Convertir nombres a mayúsculas (CP/M) */
    to_upper(argv[1]);
    to_upper(argv[2]);
    
    printf("Entrada: %s\n", argv[1]);
    printf("Salida: %s\n", argv[2]);
    printf("Frame: %d ms\n\n", frame_ms);
    
    /* Parsear WAV */
    if (!parse_wav(argv[1], frame_ms)) {
        return 1;
    }
    
    /* Generar ASM */
    generate_asm(argv[2]);
    
    printf("\nListo!\n");
    printf("Ensamblalo con: ASM %s\n", argv[2]);
    
    return 0;
}
