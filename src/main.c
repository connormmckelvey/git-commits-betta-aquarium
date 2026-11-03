#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generate.h"

int main(int argc, char** argv) {
    /* 1. Start with default generation arguments */
    GenerateArgs args = default_generate_args();

    /* 2. Parse command-line flags */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--theme") == 0 && i + 1 < argc) {
            args.theme = argv[++i];
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            args.seed = argv[++i];
        } else if (strcmp(argv[i], "--plumpness") == 0 && i + 1 < argc) {
            args.plumpness = (float)atof(argv[++i]);
        } else if (strcmp(argv[i], "--fin-scale") == 0 && i + 1 < argc) {
            args.fin_scale = (float)atof(argv[++i]);
        } else if (strcmp(argv[i], "--no-fins") == 0) {
            args.show_fins = 0;
        } else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            args.scale = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--blink") == 0) {
            args.eye_blink = 1;
        } else if (strcmp(argv[i], "--mouth-open") == 0) {
            args.mouth_open = 1;
        } else if (strcmp(argv[i], "--flare") == 0) {
            args.gill_flare = 1;
        } else if (strcmp(argv[i], "--animated") == 0) {
            args.animated = 1;
        } else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            args.frames = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) {
            args.fps = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--aquarium") == 0) {
            args.aquarium = 1;
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            args.aquarium_mode = argv[++i];
        } else if (strcmp(argv[i], "--stream") == 0) {
            args.aquarium_mode = "stream";
        } else if (strcmp(argv[i], "--cross") == 0) {
            args.aquarium_mode = "cross";
        } else if (strcmp(argv[i], "--fish-count") == 0 && i + 1 < argc) {
            args.fish_count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--tank-width") == 0 && i + 1 < argc) {
            args.tank_width = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--tank-height") == 0 && i + 1 < argc) {
            args.tank_height = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--repo") == 0 && i + 1 < argc) {
            args.repo = argv[++i];
        } else if (strcmp(argv[i], "--commits") == 0 && i + 1 < argc) {
            args.commits = atoi(argv[++i]);
        } else if ((strcmp(argv[i], "--languages") == 0 || strcmp(argv[i], "--langs") == 0) && i + 1 < argc) {
            args.languages = argv[++i];
        } else if (strcmp(argv[i], "--grow") == 0) {
            args.grow_animation = 1;
            args.animated = 1;
        } else if (strcmp(argv[i], "--fish-commits") == 0 && i + 1 < argc) {
            args.fish_commits = argv[++i];
        } else if (strcmp(argv[i], "--fish-languages") == 0 && i + 1 < argc) {
            args.fish_languages = argv[++i];
        } else if (strcmp(argv[i], "--fish-stars") == 0 && i + 1 < argc) {
            args.fish_stars = argv[++i];
        } else if (strcmp(argv[i], "--fish-recent") == 0 && i + 1 < argc) {
            args.fish_recent = argv[++i];
        } else if ((strcmp(argv[i], "--lighting") == 0 || strcmp(argv[i], "--theme-mode") == 0) && i + 1 < argc) {
            args.lighting_theme = argv[++i];
        } else if (strcmp(argv[i], "--seeds") == 0 && i + 1 < argc) {
            args.fish_seeds = argv[++i];
        } else if (strcmp(argv[i], "--themes") == 0 && i + 1 < argc) {
            args.fish_themes = argv[++i];
        } else if (strcmp(argv[i], "--out-gif") == 0 && i + 1 < argc) {
            args.out_gif = argv[++i];
        } else if (strcmp(argv[i], "--out-bmp") == 0 && i + 1 < argc) {
            args.out_bmp = argv[++i];
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            args.out_gif = argv[++i];
        }
    }

    /* 3. Run the unified generate function */
    return generate(&args);
}
