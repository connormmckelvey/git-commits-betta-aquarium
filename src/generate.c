#include "generate.h"
#include "aquarium.h"
#include "repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GenerateArgs default_generate_args(void) {
    GenerateArgs a;
    memset(&a, 0, sizeof(a));
    a.theme          = "sapphire";
    a.seed           = "git-betta";
    a.plumpness      = 1.0f;
    a.fin_scale      = 1.0f;
    a.show_fins      = 1;
    a.mouth_open     = 0;
    a.eye_blink      = 0;
    a.gill_flare     = 0;
    a.scale          = DEFAULT_SCALE;
    a.out_gif        = "dist/fish.gif";
    a.out_bmp        = "dist/fish.bmp";
    a.animated       = 0;
    a.frames         = 0; /* 0: auto-select based on mode (24 for single-fish, 48 for cross, 36 for stream) */
    a.fps            = 0; /* 0: auto-select based on mode (12 for single-fish, 16 for aquarium) */
    a.repo           = NULL;
    a.commits        = 0;
    a.languages      = NULL;
    a.grow_animation = 0;
    a.aquarium       = 0;
    a.aquarium_mode  = "cross";
    a.tank_width     = 320;
    a.tank_height    = 180;
    a.fish_count     = 3;
    a.fish_seeds     = NULL;
    a.fish_themes    = NULL;
    a.fish_commits   = NULL;
    a.fish_languages = NULL;
    return a;
}

static PaletteTheme parse_theme(const char* name) {
    if (!name) return THEME_SAPPHIRE;
    if (strcmp(name, "emerald") == 0) return THEME_EMERALD;
    if (strcmp(name, "flame") == 0)   return THEME_FLAME;
    if (strcmp(name, "cyan") == 0)    return THEME_CYAN;
    return THEME_SAPPHIRE;
}

int generate(const GenerateArgs* args) {
    GenerateArgs actual;
    if (!args) {
        actual = default_generate_args();
    } else {
        actual = *args;
        if (!actual.theme) actual.theme = "sapphire";
        if (!actual.seed) actual.seed = "git-betta";
        if (actual.plumpness <= 0.0f) actual.plumpness = 1.0f;
        if (actual.scale <= 0) actual.scale = DEFAULT_SCALE;
    }

    /* 1. Route to Multi-Fish Aquarium if requested */
    if (actual.aquarium) {
        return generate_aquarium(&actual);
    }

    /* If grow animation requested, ensure animated flag is on */
    if (actual.grow_animation) {
        actual.animated = 1;
        if (actual.frames <= 0) actual.frames = 48;
        if (actual.fps <= 0) actual.fps = 12;
    }

    /* Single fish defaults */
    if (actual.frames <= 0) actual.frames = 24;
    if (actual.fps <= 0) actual.fps = 12;

    /* 2. Check if GitHub Repo Mode is active */
    int is_repo_mode = (actual.repo != NULL || actual.commits > 0 || actual.languages != NULL || actual.grow_animation);
    RepoProfile repo_profile;
    memset(&repo_profile, 0, sizeof(repo_profile));

    if (is_repo_mode) {
        const char* rname = actual.repo ? actual.repo : actual.seed;
        int commits = (actual.commits > 0) ? actual.commits : (actual.grow_animation ? 1000 : 50);
        repo_profile = repo_profile_parse(rname, commits, actual.languages);
        actual.seed = repo_profile.repo_name;
    }

    /* 3. Compute Continuous Genetic Traits from Seed Text (controls shape/style, NOT color) */
    BettaTraits traits = betta_traits_from_seed(actual.seed);

    if (is_repo_mode) {
        GrowthStage stage = repo_growth_stage(repo_profile.commits);
        printf("GitHub Repository Pet: \"%s\"\n", repo_profile.repo_name);
        printf("  Commits: %d | Growth: %s (Factor: %.2f)\n",
               repo_profile.commits, repo_growth_stage_name(stage),
               repo_growth_factor(repo_profile.commits));
        printf("  Languages (%d): ", repo_profile.language_count);
        for (int i = 0; i < repo_profile.language_count; i++) {
            printf("%s (%.0f%%) ", repo_profile.languages[i].name, repo_profile.languages[i].percentage);
        }
        printf("\n");

        if (!actual.grow_animation) {
            betta_grow(&traits, repo_profile.commits);
        }
    } else {
        printf("Procedural Betta [Seed: \"%s\", Theme: %s]\n", traits.seed_name, actual.theme);
    }

    printf("  Morphology: %s (%dpx, %s) | %s snout | %s eye\n",
           body_archetype_name(traits.body_archetype), traits.body_length,
           body_pattern_name(traits.body_pattern),
           snout_style_name(traits.snout_style), eye_style_name(traits.eye_style));
    printf("  Finnage:    %s (%s) | %s | %s\n",
           fin_style_name(traits.fin_style), fin_pattern_name(traits.fin_pattern),
           pectoral_style_name(traits.pectoral_style),
           ventral_style_name(traits.ventral_style));
    printf("  Continuous: Cleft=%.2f, Ruffle=%.1fpx, Spike=%.2f, Droop=%.2f, Spade=%.2f\n",
           traits.cleft_depth, traits.ruffle_amp, traits.spike_protrusion, traits.gravity_droop, traits.spade_sharpness);

    /* 4. Configure Fish Dimensions using Traits */
    BettaBodyConfig cfg = betta_body_default_config();
    cfg.center_x    = 56;
    cfg.center_y    = (traits.gravity_droop > 0.65f) ? 50 : 54;
    cfg.length      = traits.body_length;
    cfg.plumpness   = actual.plumpness * (traits.belly_depth > 0.5f ? traits.belly_depth : 1.0f);
    cfg.mouth_open  = actual.mouth_open;
    cfg.eye_blink   = actual.eye_blink;
    cfg.gill_flare  = actual.gill_flare;
    cfg.swim_phase  = 0.0f;
    cfg.swim_amp    = 2.6f; /* Lifelike fluid body undulation matching octocat */
    cfg.facing_left = 1;
    cfg.traits      = traits;

    BettaFinConfig fin_cfg = betta_fin_default_config();
    fin_cfg.fin_scale = (actual.fin_scale > 0.0f) ? actual.fin_scale : 1.0f;
    fin_cfg.traits    = traits;

    /* 5. Initialize Grid & Palette (Repo Multi-Color or Theme) */
    PixelGrid grid;
    if (is_repo_mode) {
        palette_from_repo(&grid, &repo_profile);
    } else {
        PaletteTheme theme = parse_theme(actual.theme);
        palette_load_theme(&grid, theme);
    }

    /* 6. Animated Single-Fish GIF Rendering */
    if (actual.animated && actual.out_gif) {
        int num_frames = actual.frames;
        int delay_cs = 100 / actual.fps;
        int out_w = GRID_WIDTH * actual.scale;
        int out_h = GRID_HEIGHT * actual.scale;

        uint8_t* rgba = (uint8_t*)malloc(out_w * out_h * 4);
        if (!rgba) return -2;

        GifStream* stream = gif_stream_open(actual.out_gif, out_w, out_h, delay_cs);
        if (!stream) {
            free(rgba);
            fprintf(stderr, "Error opening GIF output: %s\n", actual.out_gif);
            return -3;
        }

        if (actual.grow_animation) {
            printf("Generating Growth Time-Lapse GIF: 1 to %d commits (%d frames @ %d fps)...\n",
                   repo_profile.commits, num_frames, actual.fps);
        } else {
            printf("Generating seamless animated swim loop (%d frames @ %d fps)...\n",
                   num_frames, actual.fps);
        }

        for (int frame = 0; frame < num_frames; frame++) {
            float phase = 2.0f * (float)M_PI * (float)frame / (float)num_frames;
            cfg.swim_phase = phase;
            fin_cfg.flow_phase = phase;

            /* If growth time-lapse animation, scale fish traits with frame index */
            if (actual.grow_animation) {
                float t = (float)frame / (float)(num_frames - 1);
                /* Hold fully grown mature form for final 15% of frames */
                float grow_t = (t < 0.85f) ? (t / 0.85f) : 1.0f;
                int cur_commits = (int)roundf(1.0f + ((float)repo_profile.commits - 1.0f) * powf(grow_t, 1.8f));
                if (cur_commits < 1) cur_commits = 1;

                BettaTraits frame_traits = betta_traits_from_seed(actual.seed);
                betta_grow(&frame_traits, cur_commits);

                cfg.traits = frame_traits;
                cfg.length = frame_traits.body_length;
                cfg.plumpness = actual.plumpness * (frame_traits.belly_depth > 0.5f ? frame_traits.belly_depth : 1.0f);
                cfg.center_y = (frame_traits.gravity_droop > 0.65f) ? 50 : 54;
                fin_cfg.traits = frame_traits;
            }

            grid_init(&grid, PAL_BG);

            if (actual.show_fins) {
                render_betta_fins_bg(&grid, &cfg, &fin_cfg);
            }
            render_betta_body(&grid, &cfg);
            if (actual.show_fins) {
                render_betta_fins_fg(&grid, &cfg, &fin_cfg);
            }

            /* Upscale to output RGBA buffer */
            for (int y = 0; y < out_h; y++) {
                int src_y = y / actual.scale;
                for (int x = 0; x < out_w; x++) {
                    int src_x = x / actual.scale;
                    uint8_t idx = grid.pixels[src_y][src_x];
                    Color c = grid.palette[idx];
                    int i = (y * out_w + x) * 4;
                    rgba[i + 0] = c.r;
                    rgba[i + 1] = c.g;
                    rgba[i + 2] = c.b;
                    rgba[i + 3] = c.a;
                }
            }

            gif_stream_write_frame(stream, rgba);

            /* Save frame 0 as BMP if requested */
            if (frame == 0 && actual.out_bmp) {
                grid_save_bmp(&grid, actual.out_bmp, actual.scale);
                printf("Saved initial static frame to BMP: %s\n", actual.out_bmp);
            }
        }

        gif_stream_close(stream);
        free(rgba);
        printf("Successfully saved Animated GIF: %s (%dx%d, %d frames)\n",
               actual.out_gif, out_w, out_h, num_frames);
        return 0;
    }

    /* 7. Static Image Rendering (Single Frame) */
    grid_init(&grid, PAL_BG);

    if (actual.show_fins) {
        render_betta_fins_bg(&grid, &cfg, &fin_cfg);
    }
    render_betta_body(&grid, &cfg);
    if (actual.show_fins) {
        render_betta_fins_fg(&grid, &cfg, &fin_cfg);
    }

    /* Export BMP */
    if (actual.out_bmp) {
        int res_bmp = grid_save_bmp(&grid, actual.out_bmp, actual.scale);
        if (res_bmp == 0) {
            printf("Saved BMP to: %s\n", actual.out_bmp);
        } else {
            fprintf(stderr, "Error saving BMP to %s (code %d)\n", actual.out_bmp, res_bmp);
        }
    }

    /* Export Static GIF */
    if (actual.out_gif) {
        int res_gif = grid_save_gif(&grid, actual.out_gif, actual.scale);
        if (res_gif == 0) {
            printf("Saved GIF to: %s\n", actual.out_gif);
        } else {
            fprintf(stderr, "Error saving GIF to %s (code %d)\n", actual.out_gif, res_gif);
        }
    }

    return 0;
}
