#ifndef PIXEL_GRID_H
#define PIXEL_GRID_H

#include <stdint.h>
#include <stddef.h>

#define GRID_WIDTH    160
#define GRID_HEIGHT   120
#define DEFAULT_SCALE 2

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

typedef struct {
    uint8_t pixels[GRID_HEIGHT][GRID_WIDTH];
    Color palette[32];
    uint8_t bg_index;
} PixelGrid;

/* Initialize grid with a background color index */
void grid_init(PixelGrid* g, uint8_t bg_index);

/* Set pixel if inside bounds */
void grid_set_pixel(PixelGrid* g, int x, int y, uint8_t color_idx);

/* Get pixel, returns bg_index if out of bounds */
uint8_t grid_get_pixel(const PixelGrid* g, int x, int y);

/* Save grid as an uncompressed 24-bit BMP image upscaled by `scale` factor */
int grid_save_bmp(const PixelGrid* g, const char* filepath, int scale);

/* Save grid as a static GIF image upscaled by `scale` factor */
int grid_save_gif(const PixelGrid* g, const char* filepath, int scale);

/* Opaque handle for multi-frame animated GIF streaming */
typedef struct GifStream GifStream;

/* Open a new animated GIF for streaming frames */
GifStream* gif_stream_open(const char* filepath, int width, int height, int delay_cs);

/* Write a frame of size (width * height * 4) RGBA bytes */
int gif_stream_write_frame(GifStream* stream, const uint8_t* rgba);

/* Finish and close the animated GIF */
void gif_stream_close(GifStream* stream);

#endif /* PIXEL_GRID_H */
