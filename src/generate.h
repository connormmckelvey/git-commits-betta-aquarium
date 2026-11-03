#ifndef GENERATE_H
#define GENERATE_H

#include "pixel_grid.h"
#include "palette.h"
#include "betta_body.h"
#include "betta_fins.h"

/* Arguments for generating the Betta fish */
typedef struct {
    const char* theme;            /* "sapphire", "emerald", "flame", "cyan" (controls colors) */
    const char* seed;             /* Text seed for morphology/fins/expression (does NOT affect color) */
    float       plumpness;        /* 0.8 (sleek) to 1.3 (plump), default 1.0f */
    float       fin_scale;        /* Fin size multiplier (default 1.0f) */
    int         show_fins;        /* 1 to render flowing fins, 0 for body only (default 1) */
    int         mouth_open;       /* 1: open feeding mouth, 0: closed */
    int         eye_blink;        /* 1: closed sleeping/blinking eye, 0: open */
    int         gill_flare;       /* 1: flaring operculum gills, 0: normal */
    int         scale;            /* Nearest-neighbor upscale factor (default: 2) */
    const char* out_gif;          /* Output GIF path (e.g. "dist/fish.gif") or NULL */
    const char* out_bmp;          /* Output BMP path (e.g. "dist/fish.bmp") or NULL */

    /* Kinematic Animation (Single Fish) */
    int         animated;         /* 1 to generate animated swimming GIF, 0 for static */
    int         frames;           /* Number of loop frames (default: 16) */
    int         fps;              /* Playback frames per second (default: 16) */

    /* GitHub Repository & Multi-Color Growth */
    const char* repo;             /* GitHub repository name / seed (e.g. "torvalds/linux") */
    int         commits;          /* Total commit count (controls morphological growth) */
    const char* languages;        /* Language breakdown (e.g. "Python:60,Rust:30,Shell:10") */
    int         grow_animation;   /* 1 to generate animated growth time-lapse GIF */

    /* Multi-Fish Aquarium Mode */
    int         aquarium;         /* 1 for multi-fish aquarium simulation */
    const char* aquarium_mode;    /* "cross" (off-screen turnaround, default) or "stream" (upstream hover) */
    int         tank_width;       /* Aquarium tank width (default: 240) */
    int         tank_height;      /* Aquarium tank height (default: 160) */
    int         fish_count;       /* Number of fish swimming together (2 to 6, default 3) */
    const char* fish_seeds;       /* Comma-separated seeds or NULL */
    const char* fish_themes;      /* Comma-separated themes or NULL */
    const char* fish_commits;     /* Comma-separated commit counts for aquarium */
    const char* fish_languages;   /* Semicolon-separated language specs for aquarium */
} GenerateArgs;

/* Return default configuration args */
GenerateArgs default_generate_args(void);

/* One unified function that encapsulates all generation pipelines:
 * - Static single-fish image
 * - Animated swimming single-fish GIF
 * - Multi-fish animated aquarium simulation
 * Returns 0 on success, non-zero on error.
 */
int generate(const GenerateArgs* args);

#endif /* GENERATE_H */
