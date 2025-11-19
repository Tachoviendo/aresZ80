#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <mpg123.h>

// Configuración para Z80
#define TARGET_SAMPLE_RATE 8000   // 8kHz - ajustable según tu Z80
#define TARGET_CHANNELS 1          // Mono

// Estructura para el header WAV
typedef struct {
    char riff[4];
    uint32_t file_size;
    char wave[4];
    char fmt[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t data_size;
} __attribute__((packed)) WAVHeader;

// Convertir sample de 16 bits a 8 bits sin signo (0-255)
uint8_t convert_to_8bit(int16_t sample_16bit) {
    int32_t temp = sample_16bit + 32768;
    return (uint8_t)(temp >> 8);
}

// Convertir stereo a mono promediando canales
int16_t stereo_to_mono(int16_t left, int16_t right) {
    return (left / 2) + (right / 2);
}

void write_wav_header(FILE *fp, int sample_rate, int num_channels, size_t num_samples) {
    WAVHeader header;
    
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    memcpy(header.data, "data", 4);
    
    header.fmt_size = 16;
    header.audio_format = 1;
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.bits_per_sample = 8;
    header.block_align = num_channels;
    header.byte_rate = sample_rate * header.block_align;
    header.data_size = num_samples * num_channels;
    header.file_size = 36 + header.data_size;
    
    fwrite(&header, sizeof(WAVHeader), 1, fp);
}

int main(int argc, char *argv[]) {
    int target_rate = TARGET_SAMPLE_RATE;
    int save_raw = 0;  // Para guardar formato raw sin header
    
    if (argc < 3) {
        printf("Uso: %s <entrada.mp3> <salida> [opciones]\n", argv[0]);
        printf("\nOpciones:\n");
        printf("  -r <rate>    Sample rate (default: 8000 Hz)\n");
        printf("  -raw         Guardar formato RAW sin header WAV\n");
        printf("  -h           Incluir header C para tu código Z80\n");
        printf("\nEjemplos:\n");
        printf("  %s song.mp3 song.wav\n", argv[0]);
        printf("  %s song.mp3 song.raw -raw -r 11025\n", argv[0]);
        printf("  %s song.mp3 song.h -h\n", argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    int generate_header = 0;
    
    // Parsear opciones
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            target_rate = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-raw") == 0) {
            save_raw = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            generate_header = 1;
            save_raw = 1;  // Header implica raw
        }
    }
    
    printf("=== Conversor MP3 para Z80 ===\n");
    printf("Entrada: %s\n", input_file);
    printf("Salida: %s\n", output_file);
    printf("Target sample rate: %d Hz\n", target_rate);
    printf("Formato: %s\n", save_raw ? "RAW (sin header)" : "WAV");
    printf("\n");
    
    // Inicializar mpg123
    mpg123_handle *mh;
    int err;
    long rate;
    int channels, encoding;
    
    err = mpg123_init();
    if (err != MPG123_OK) {
        fprintf(stderr, "Error inicializando mpg123: %s\n", mpg123_plain_strerror(err));
        return 1;
    }
    
    mh = mpg123_new(NULL, &err);
    if (!mh) {
        fprintf(stderr, "Error creando handle: %s\n", mpg123_plain_strerror(err));
        return 1;
    }
    
    if (mpg123_open(mh, input_file) != MPG123_OK) {
        fprintf(stderr, "Error abriendo archivo: %s\n", mpg123_strerror(mh));
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }
    
    // Obtener formato original
    mpg123_getformat(mh, &rate, &channels, &encoding);
    
    printf("Audio original:\n");
    printf("  Sample rate: %ld Hz\n", rate);
    printf("  Canales: %d\n", channels);
    
    // Forzar formato de salida
    mpg123_format_none(mh);
    mpg123_format(mh, target_rate, TARGET_CHANNELS, MPG123_ENC_SIGNED_16);
    
    // Abrir archivo de salida
    FILE *out_fp = fopen(output_file, "wb");
    if (!out_fp) {
        fprintf(stderr, "Error creando archivo de salida\n");
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 1;
    }
    
    // Escribir header WAV si no es raw
    if (!save_raw) {
        write_wav_header(out_fp, target_rate, TARGET_CHANNELS, 0);
    }
    
    // Buffer para leer datos
    size_t buffer_size = mpg123_outblock(mh);
    unsigned char *buffer = malloc(buffer_size);
    size_t total_samples = 0;
    size_t done;
    
    printf("\nProcesando...\n");
    
    // Procesar audio
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK) {
        int16_t *samples_16 = (int16_t *)buffer;
        size_t num_samples = done / 2;
        
        for (size_t i = 0; i < num_samples; i++) {
            uint8_t sample_8 = convert_to_8bit(samples_16[i]);
            fwrite(&sample_8, 1, 1, out_fp);
        }
        
        total_samples += num_samples;
    }
    
    printf("Samples totales: %zu\n", total_samples);
    printf("Duración: %.2f segundos\n", (float)total_samples / target_rate);
    printf("Tamaño: %zu bytes\n", total_samples);
    
    // Si no es raw, actualizar header con tamaño correcto
    if (!save_raw) {
        fseek(out_fp, 0, SEEK_SET);
        write_wav_header(out_fp, target_rate, TARGET_CHANNELS, total_samples);
    }
    
    fclose(out_fp);
    
    // Generar header C si se solicitó
    if (generate_header) {
        char header_file[256];
        snprintf(header_file, sizeof(header_file), "%s.h", output_file);
        
        FILE *h_fp = fopen(header_file, "w");
        if (h_fp) {
            fprintf(h_fp, "// Audio data for Z80\n");
            fprintf(h_fp, "// Generated from: %s\n", input_file);
            fprintf(h_fp, "// Sample rate: %d Hz\n", target_rate);
            fprintf(h_fp, "// Format: 8-bit unsigned PCM, mono\n\n");
            
            // Extraer nombre base del archivo
            char base_name[256];
            const char *last_slash = strrchr(output_file, '/');
            const char *name_start = last_slash ? last_slash + 1 : output_file;
            strncpy(base_name, name_start, sizeof(base_name) - 1);
            base_name[sizeof(base_name) - 1] = '\0';  // Asegurar null-termination
            char *dot = strrchr(base_name, '.');
            if (dot) *dot = '\0';
            
            // Reemplazar caracteres no válidos
            for (char *p = base_name; *p; p++) {
                if (!isalnum(*p)) *p = '_';
            }
            
            fprintf(h_fp, "#ifndef AUDIO_%s_H\n", base_name);
            fprintf(h_fp, "#define AUDIO_%s_H\n\n", base_name);
            fprintf(h_fp, "#define AUDIO_SAMPLE_RATE %d\n", target_rate);
            fprintf(h_fp, "#define AUDIO_LENGTH %zu\n\n", total_samples);
            fprintf(h_fp, "extern const unsigned char audio_data[%zu];\n\n", total_samples);
            fprintf(h_fp, "#endif\n");
            
            fclose(h_fp);
            printf("\nHeader C generado: %s\n", header_file);
        }
    }
    
    // Limpiar
    free(buffer);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    
    printf("\n¡Conversión completada!\n");
    printf("\nPara usar en tu Z80:\n");
    printf("  1. El audio está en formato 8-bit unsigned (0-255)\n");
    printf("  2. Sample rate: %d Hz\n", target_rate);
    printf("  3. Mono (1 canal)\n");
    printf("  4. Tamaño total: %zu bytes\n", total_samples);
    
    if (save_raw) {
        printf("\nFormato RAW: Los bytes están listos para enviar directo al DAC\n");
    }
    
    return 0;
}
