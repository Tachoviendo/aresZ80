/*
 * WAV2SID.C - Conversor de WAV a código ASM para SIDulator RC2014
 * 
 * Uso: ./wav2sid input.wav output.asm [duration_ms]
 * 
 * Requisitos:
 *   - Audio WAV 8-bit mono
 *   - Frecuencias: 8000 Hz o 11025 Hz recomendado
 * 
 * Compilar: gcc -o wav2sid wav2sid.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define MAX_NOTES 2000
#define SID_PORT_BASE 0xD4
#define SID_PORT_REG  0xD4
#define SID_PORT_DATA 0xD5

/* Estructura WAV simplificada */
typedef struct {
    char riff[4];           // "RIFF"
    uint32_t size;
    char wave[4];           // "WAVE"
} WAVHeader;

typedef struct {
    char fmt[4];            // "fmt "
    uint32_t size;
    uint16_t format;        // 1 = PCM
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WAVFmt;

typedef struct {
    char data[4];           // "data"
    uint32_t size;
} WAVData;

typedef struct {
    uint16_t freq;
    uint8_t volume;
    uint16_t duration;
} SIDNote;

SIDNote notes[MAX_NOTES];
int note_count = 0;

/* Detectar frecuencia dominante usando análisis de cruces por cero simplificado */
float detect_frequency(uint8_t *samples, int n_samples, int sample_rate) {
    int zero_crossings = 0;
    int prev_sign = (samples[0] >= 128) ? 1 : -1;
    
    for (int i = 1; i < n_samples; i++) {
        int curr_sign = (samples[i] >= 128) ? 1 : -1;
        if (curr_sign != prev_sign) {
            zero_crossings++;
        }
        prev_sign = curr_sign;
    }
    
    /* Frecuencia = (cruces por cero / 2) / tiempo */
    float time = (float)n_samples / sample_rate;
    float freq = (zero_crossings / 2.0f) / time;
    
    return freq;
}

/* Calcular volumen promedio */
uint8_t calculate_volume(uint8_t *samples, int n_samples) {
    long sum = 0;
    for (int i = 0; i < n_samples; i++) {
        int val = samples[i] - 128;  // Centrar en 0
        sum += abs(val);
    }
    
    int avg = sum / n_samples;
    
    /* Mapear a rango 0-15 del SID */
    int volume = (avg * 15) / 128;
    if (volume > 15) volume = 15;
    
    return (uint8_t)volume;
}

/* Convertir frecuencia Hz a registro SID (16 bits) */
uint16_t freq_to_sid(float freq_hz) {
    /* Fórmula del SID: freq_reg = (freq_hz * 16777216) / clock_freq
     * Para clock de 1 MHz: freq_reg = freq_hz * 16.777216
     * Simplificado: freq_reg ≈ freq_hz * 17
     */
    uint32_t sid_freq = (uint32_t)(freq_hz * 16.777216);
    
    if (sid_freq > 65535) sid_freq = 65535;
    if (sid_freq < 0) sid_freq = 0;
    
    return (uint16_t)sid_freq;
}

/* Parsear archivo WAV */
int parse_wav(const char *filename, uint8_t **audio_data, int *sample_rate, 
              int *n_samples, int duration_ms) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: No se puede abrir %s\n", filename);
        return 0;
    }
    
    WAVHeader header;
    WAVFmt fmt;
    WAVData data_header;
    
    /* Leer cabecera RIFF */
    fread(&header, sizeof(WAVHeader), 1, fp);
    if (strncmp(header.riff, "RIFF", 4) != 0 || 
        strncmp(header.wave, "WAVE", 4) != 0) {
        fprintf(stderr, "Error: No es un archivo WAV válido\n");
        fclose(fp);
        return 0;
    }
    
    /* Leer formato */
    fread(&fmt, sizeof(WAVFmt), 1, fp);
    if (strncmp(fmt.fmt, "fmt ", 4) != 0) {
        fprintf(stderr, "Error: Formato WAV inválido\n");
        fclose(fp);
        return 0;
    }
    
    /* Verificar formato */
    if (fmt.format != 1) {
        fprintf(stderr, "Error: Solo se soporta PCM (format=%d)\n", fmt.format);
        fclose(fp);
        return 0;
    }
    
    if (fmt.channels != 1) {
        fprintf(stderr, "Error: Solo se soporta mono (channels=%d)\n", fmt.channels);
        fclose(fp);
        return 0;
    }
    
    if (fmt.bits_per_sample != 8) {
        fprintf(stderr, "Error: Solo se soporta 8 bits (bits=%d)\n", fmt.bits_per_sample);
        fclose(fp);
        return 0;
    }
    
    /* Buscar chunk de datos */
    while (1) {
        fread(&data_header, sizeof(WAVData), 1, fp);
        if (strncmp(data_header.data, "data", 4) == 0) {
            break;
        }
        /* Saltar chunk desconocido */
        fseek(fp, data_header.size, SEEK_CUR);
    }
    
    /* Leer datos de audio */
    *n_samples = data_header.size;
    *sample_rate = fmt.sample_rate;
    
    *audio_data = (uint8_t *)malloc(*n_samples);
    if (!*audio_data) {
        fprintf(stderr, "Error: No hay memoria suficiente\n");
        fclose(fp);
        return 0;
    }
    
    fread(*audio_data, 1, *n_samples, fp);
    fclose(fp);
    
    printf("WAV cargado: %d Hz, %d samples, %.2f segundos\n",
           *sample_rate, *n_samples, (float)*n_samples / *sample_rate);
    
    /* Analizar en frames */
    int frame_samples = (*sample_rate * duration_ms) / 1000;
    int total_frames = *n_samples / frame_samples;
    
    printf("Duración de frame: %d ms (%d samples/frame)\n", 
           duration_ms, frame_samples);
    printf("Frames totales: %d\n", total_frames);
    
    note_count = 0;
    
    for (int i = 0; i < total_frames && note_count < MAX_NOTES; i++) {
        int offset = i * frame_samples;
        uint8_t *frame = *audio_data + offset;
        
        /* Detectar frecuencia y volumen */
        float freq_hz = detect_frequency(frame, frame_samples, *sample_rate);
        uint8_t volume = calculate_volume(frame, frame_samples);
        
        /* Filtrar ruido */
        if (volume < 2) {
            freq_hz = 0;
            volume = 0;
        }
        
        /* Convertir a registro SID */
        uint16_t sid_freq = freq_to_sid(freq_hz);
        
        notes[note_count].freq = sid_freq;
        notes[note_count].volume = volume;
        notes[note_count].duration = duration_ms;
        note_count++;
    }
    
    printf("Generadas %d notas\n", note_count);
    
    return 1;
}

/* Generar código ASM */
void generate_asm(const char *output_file) {
    FILE *fp = fopen(output_file, "w");
    if (!fp) {
        fprintf(stderr, "Error: No se puede crear %s\n", output_file);
        return;
    }
    
    /* Cabecera */
    fprintf(fp, "; Código generado por WAV2SID\n");
    fprintf(fp, "; %d notas de audio\n", note_count);
    fprintf(fp, "; Puerto SID: $%02X (REG), $%02X (DATA)\n\n", 
            SID_PORT_REG, SID_PORT_DATA);
    
    fprintf(fp, "\tORG $0100\n\n");
    
    /* Entry point */
    fprintf(fp, "START:\n");
    fprintf(fp, "\tCALL INIT_SID\n");
    fprintf(fp, "\tCALL PLAY_SONG\n");
    fprintf(fp, "\tRST 0\t\t\t; Volver a CP/M\n\n");
    
    /* Inicialización del SID */
    fprintf(fp, "; Inicializar SID\n");
    fprintf(fp, "INIT_SID:\n");
    fprintf(fp, "\t; Limpiar todos los registros\n");
    fprintf(fp, "\tLD B, 29\t\t; 29 registros del SID\n");
    fprintf(fp, "\tLD C, 0\t\t\t; Valor 0\n");
    fprintf(fp, "\tLD A, 0\t\t\t; Registro inicial\n");
    fprintf(fp, "CLEAR_LOOP:\n");
    fprintf(fp, "\tOUT ($%02X), A\t\t; Seleccionar registro\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, C\n");
    fprintf(fp, "\tOUT ($%02X), A\t\t; Escribir 0\n", SID_PORT_DATA);
    fprintf(fp, "\tLD A, B\n");
    fprintf(fp, "\tDEC A\n");
    fprintf(fp, "\tLD B, A\n");
    fprintf(fp, "\tOR A\n");
    fprintf(fp, "\tJR NZ, CLEAR_LOOP\n\n");
    
    fprintf(fp, "\t; Configurar Voice 1 - Triangle wave\n");
    fprintf(fp, "\tLD A, $04\t\t; Registro 4 (Control)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, $11\t\t; Triangle wave + Gate ON\n");
    fprintf(fp, "\tOUT ($%02X), A\n\n", SID_PORT_DATA);
    
    fprintf(fp, "\t; Configurar ADSR\n");
    fprintf(fp, "\tLD A, $05\t\t; Registro 5 (Attack/Decay)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, $09\t\t; Attack=0, Decay=9\n");
    fprintf(fp, "\tOUT ($%02X), A\n\n", SID_PORT_DATA);
    
    fprintf(fp, "\tLD A, $06\t\t; Registro 6 (Sustain/Release)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, $F0\t\t; Sustain=15, Release=0\n");
    fprintf(fp, "\tOUT ($%02X), A\n\n", SID_PORT_DATA);
    
    fprintf(fp, "\tRET\n\n");
    
    /* Reproducción */
    fprintf(fp, "; Reproducir canción\n");
    fprintf(fp, "PLAY_SONG:\n");
    fprintf(fp, "\tLD HL, NOTE_DATA\n");
    fprintf(fp, "\tLD B, %d\t\t; Número de notas\n", note_count);
    fprintf(fp, "PLAY_LOOP:\n");
    fprintf(fp, "\tPUSH BC\n\n");
    
    fprintf(fp, "\t; Leer frecuencia LO\n");
    fprintf(fp, "\tLD A, $00\t\t; Registro 0 (Freq LO)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_DATA);
    fprintf(fp, "\tINC HL\n\n");
    
    fprintf(fp, "\t; Leer frecuencia HI\n");
    fprintf(fp, "\tLD A, $01\t\t; Registro 1 (Freq HI)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_DATA);
    fprintf(fp, "\tINC HL\n\n");
    
    fprintf(fp, "\t; Leer volumen\n");
    fprintf(fp, "\tLD A, $18\t\t; Registro 24 (Volumen global)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_REG);
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tOUT ($%02X), A\n", SID_PORT_DATA);
    fprintf(fp, "\tINC HL\n\n");
    
    fprintf(fp, "\t; Delay\n");
    fprintf(fp, "\tLD A, (HL)\n");
    fprintf(fp, "\tINC HL\n");
    fprintf(fp, "\tCALL DELAY_MS\n\n");
    
    fprintf(fp, "\tPOP BC\n");
    fprintf(fp, "\tDJNZ PLAY_LOOP\n");
    fprintf(fp, "\tRET\n\n");
    
    /* Rutina de delay */
    fprintf(fp, "; Delay en milisegundos (A = ms)\n");
    fprintf(fp, "DELAY_MS:\n");
    fprintf(fp, "\tOR A\n");
    fprintf(fp, "\tRET Z\n");
    fprintf(fp, "\tPUSH BC\n");
    fprintf(fp, "\tLD B, A\n");
    fprintf(fp, "DELAY_OUTER:\n");
    fprintf(fp, "\tPUSH BC\n");
    fprintf(fp, "\tLD BC, 400\t\t; ~1ms a 4MHz\n");
    fprintf(fp, "DELAY_INNER:\n");
    fprintf(fp, "\tDEC BC\n");
    fprintf(fp, "\tLD A, B\n");
    fprintf(fp, "\tOR C\n");
    fprintf(fp, "\tJR NZ, DELAY_INNER\n");
    fprintf(fp, "\tPOP BC\n");
    fprintf(fp, "\tDJNZ DELAY_OUTER\n");
    fprintf(fp, "\tPOP BC\n");
    fprintf(fp, "\tRET\n\n");
    
    /* Tabla de datos */
    fprintf(fp, "; Tabla de notas: freq_lo, freq_hi, volume, duration\n");
    fprintf(fp, "NOTE_DATA:\n");
    
    for (int i = 0; i < note_count; i++) {
        uint8_t freq_lo = notes[i].freq & 0xFF;
        uint8_t freq_hi = (notes[i].freq >> 8) & 0xFF;
        
        fprintf(fp, "\tDB $%02X, $%02X, $%02X, %d", 
                freq_lo, freq_hi, notes[i].volume, notes[i].duration);
        fprintf(fp, "\t; #%d: F=%d V=%d\n", 
                i+1, notes[i].freq, notes[i].volume);
    }
    
    fprintf(fp, "\n");
    
    fclose(fp);
    printf("Archivo ASM generado: %s\n", output_file);
}

int main(int argc, char *argv[]) {
    uint8_t *audio_data = NULL;
    int sample_rate, n_samples;
    int duration_ms = 50;  // Por defecto 50ms
    
    printf("=== WAV2SID - Conversor WAV a SIDulator ASM ===\n\n");
    
    if (argc < 3) {
        printf("Uso: %s input.wav output.asm [duration_ms]\n", argv[0]);
        printf("\nEjemplo: %s musica.wav musica.asm 50\n\n", argv[0]);
        printf("Requisitos del WAV:\n");
        printf("  - 8 bits\n");
        printf("  - Mono\n");
        printf("  - 8000 Hz o 11025 Hz recomendado\n\n");
        return 1;
    }
    
    if (argc >= 4) {
        duration_ms = atoi(argv[3]);
        if (duration_ms < 10 || duration_ms > 500) {
            printf("Advertencia: duración fuera de rango (10-500ms), usando 50ms\n");
            duration_ms = 50;
        }
    }
    
    /* Parsear WAV */
    if (!parse_wav(argv[1], &audio_data, &sample_rate, &n_samples, duration_ms)) {
        return 1;
    }
    
    /* Generar ASM */
    generate_asm(argv[2]);
    
    /* Limpiar */
    free(audio_data);
    
    printf("\n=== Listo! ===\n");
    printf("\nPróximos pasos:\n");
    printf("  1. Ensamblar: z80asm -b %s\n", argv[2]);
    printf("  2. Copiar el .com a tu RC2014\n");
    printf("  3. Ejecutar en CP/M\n\n");
    
    return 0;
}
