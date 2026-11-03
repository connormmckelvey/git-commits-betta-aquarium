#include "pixel_grid.h"
#include "palette.h"
#include "gif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void grid_init(PixelGrid* g, uint8_t bg_index) {
    if (!g) return;
    g->bg_index = bg_index;
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            g->pixels[y][x] = bg_index;
        }
    }
}

void grid_set_pixel(PixelGrid* g, int x, int y, uint8_t color_idx) {
    if (!g) return;
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;
    g->pixels[y][x] = color_idx;
}

uint8_t grid_get_pixel(const PixelGrid* g, int x, int y) {
    if (!g) return 0;
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return g->bg_index;
    return g->pixels[y][x];
}

/* Save as standard uncompressed 24-bit BMP with nearest-neighbor scaling */
int grid_save_bmp(const PixelGrid* g, const char* filepath, int scale) {
    if (!g || !filepath || scale <= 0) return -1;

    int out_w = GRID_WIDTH * scale;
    int out_h = GRID_HEIGHT * scale;
    int row_stride = (out_w * 3 + 3) & ~3;
    uint32_t image_size = row_stride * out_h;
    uint32_t file_size = 54 + image_size;

    uint8_t header[54];
    memset(header, 0, sizeof(header));

    header[0] = 'B';
    header[1] = 'M';
    header[2] = (uint8_t)(file_size);
    header[3] = (uint8_t)(file_size >> 8);
    header[4] = (uint8_t)(file_size >> 16);
    header[5] = (uint8_t)(file_size >> 24);
    header[10] = 54;

    header[14] = 40;
    header[18] = (uint8_t)(out_w);
    header[19] = (uint8_t)(out_w >> 8);
    header[20] = (uint8_t)(out_w >> 16);
    header[21] = (uint8_t)(out_w >> 24);
    header[22] = (uint8_t)(out_h);
    header[23] = (uint8_t)(out_h >> 8);
    header[24] = (uint8_t)(out_h >> 16);
    header[25] = (uint8_t)(out_h >> 24);
    header[26] = 1;
    header[28] = 24;
    header[34] = (uint8_t)(image_size);
    header[35] = (uint8_t)(image_size >> 8);
    header[36] = (uint8_t)(image_size >> 16);
    header[37] = (uint8_t)(image_size >> 24);

    FILE* f = fopen(filepath, "wb");
    if (!f) return -2;

    fwrite(header, 1, 54, f);

    uint8_t* row_buf = (uint8_t*)malloc(row_stride);
    if (!row_buf) {
        fclose(f);
        return -3;
    }
    memset(row_buf, 0, row_stride);

    for (int y = out_h - 1; y >= 0; y--) {
        int src_y = y / scale;
        for (int x = 0; x < out_w; x++) {
            int src_x = x / scale;
            uint8_t idx = g->pixels[src_y][src_x];
            Color c = g->palette[idx];

            row_buf[x * 3 + 0] = c.b;
            row_buf[x * 3 + 1] = c.g;
            row_buf[x * 3 + 2] = c.r;
        }
        fwrite(row_buf, 1, row_stride, f);
    }

    free(row_buf);
    fclose(f);
    return 0;
}

int grid_save_gif(const PixelGrid* g, const char* filepath, int scale) {
    if (!g || !filepath || scale <= 0) return -1;
    int out_w = GRID_WIDTH * scale;
    int out_h = GRID_HEIGHT * scale;

    uint8_t* rgba = (uint8_t*)malloc(out_w * out_h * 4);
    if (!rgba) return -2;

    for (int y = 0; y < out_h; y++) {
        int src_y = y / scale;
        for (int x = 0; x < out_w; x++) {
            int src_x = x / scale;
            uint8_t idx = g->pixels[src_y][src_x];
            Color c = g->palette[idx];
            int i = (y * out_w + x) * 4;
            rgba[i + 0] = c.r;
            rgba[i + 1] = c.g;
            rgba[i + 2] = c.b;
            rgba[i + 3] = c.a;
        }
    }

    GifWriter writer;
    if (!GifBegin(&writer, filepath, out_w, out_h, 100, 8, false)) {
        free(rgba);
        return -3;
    }
    GifWriteFrame(&writer, rgba, out_w, out_h, 100, 8, false);
    GifEnd(&writer);

    free(rgba);
    return 0;
}

/* --- Multi-Frame Animated GIF Streaming --- */
struct GifStream {
    GifWriter writer;
    int width;
    int height;
    int delay_cs;
};

GifStream* gif_stream_open(const char* filepath, int width, int height, int delay_cs) {
    if (!filepath || width <= 0 || height <= 0) return NULL;
    GifStream* s = (GifStream*)malloc(sizeof(GifStream));
    if (!s) return NULL;
    s->width = width;
    s->height = height;
    s->delay_cs = (delay_cs > 0) ? delay_cs : 6;

    if (!GifBegin(&s->writer, filepath, width, height, s->delay_cs, 8, false)) {
        free(s);
        return NULL;
    }
    return s;
}

int gif_stream_write_frame(GifStream* stream, const uint8_t* rgba) {
    if (!stream || !rgba) return -1;
    return GifWriteFrame(&stream->writer, rgba, stream->width, stream->height, stream->delay_cs, 8, false) ? 0 : -2;
}

void gif_stream_close(GifStream* stream) {
    if (!stream) return;
    GifEnd(&stream->writer);
    free(stream);
}
