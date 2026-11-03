#include "aquarium.h"
#include "palette.h"
#include "repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    char        seed[64];
    char        theme[32];
    int         commits;
    char        languages[128];
    RepoProfile repo_profile;
    int         is_repo;
    BettaTraits traits;
    float       x0, y0;
    float       rx, ry;
    int         freq_x, freq_y;
    float       phase_x, phase_y, phase_z;
    int         beats_per_loop;
    float       beat_offset;
    float       scale_factor;
} FishAgent;

typedef struct {
    float x;
    float y_start;
    float speed;
    int   size;
} AmbientBubble;

static PaletteTheme parse_theme_name(const char* name) {
    if (!name) return THEME_SAPPHIRE;
    if (strcmp(name, "emerald") == 0) return THEME_EMERALD;
    if (strcmp(name, "flame") == 0)   return THEME_FLAME;
    if (strcmp(name, "cyan") == 0)    return THEME_CYAN;
    return THEME_SAPPHIRE;
}

static int split_delimiter_string(const char* input, char delim, char items[8][128], int max_items) {
    if (!input || !*input) return 0;
    int count = 0;
    const char* start = input;
    while (*start && count < max_items) {
        while (*start == ' ' || *start == delim) start++;
        if (!*start) break;
        const char* end = start;
        while (*end && *end != delim) end++;
        int len = (int)(end - start);
        if (len >= 128) len = 127;
        strncpy(items[count], start, len);
        items[count][len] = '\0';
        count++;
        start = end;
    }
    return count;
}

static int split_comma_string(const char* input, char items[8][64], int max_items) {
    if (!input || !*input) return 0;
    int count = 0;
    const char* start = input;
    while (*start && count < max_items) {
        while (*start == ' ' || *start == ',') start++;
        if (!*start) break;
        const char* end = start;
        while (*end && *end != ',') end++;
        int len = (int)(end - start);
        if (len >= 64) len = 63;
        strncpy(items[count], start, len);
        items[count][len] = '\0';
        count++;
        start = end;
    }
    return count;
}

int generate_aquarium(const GenerateArgs* args) {
    if (!args) return -1;

    int is_stream_mode = (args->aquarium_mode && strcmp(args->aquarium_mode, "stream") == 0);

    int tank_w = (args->tank_width >= 120) ? args->tank_width : 320;
    int tank_h = (args->tank_height >= 90) ? args->tank_height : 180;
    int fish_count = (args->fish_count >= 1 && args->fish_count <= 8) ? args->fish_count : 3;
    int default_frames = is_stream_mode ? 60 : 144;
    int num_frames = (args->frames >= 4) ? args->frames : default_frames;
    int default_fps = 10;
    int fps = (args->fps >= 1) ? args->fps : default_fps;
    int delay_cs = 100 / fps;
    int scale = (args->scale >= 1) ? args->scale : 2;
    const char* out_gif = args->out_gif ? args->out_gif : "dist/aquarium.gif";

    if (is_stream_mode) {
        printf("Starting Procedural Stream Aquarium Simulation (Mode: Stream - Chill Station Hover)\n");
        printf("  Tank Resolution: %dx%d (Output: %dx%d @ scale %d)\n",
               tank_w, tank_h, tank_w * scale, tank_h * scale, scale);
        printf("  Fish Population: %d fish | Frames: %d @ %d fps (Calm zen hover, silky fins)\n",
               fish_count, num_frames, fps);
    } else {
        printf("Starting Procedural Cross Aquarium Simulation (Mode: Cross - Ultra-Slow Chill Traverse)\n");
        printf("  Tank Resolution: %dx%d (Output: %dx%d @ scale %d)\n", tank_w, tank_h, tank_w * scale, tank_h * scale, scale);

        printf("  Fish Population: %d fish | Frames: %d @ %d fps (Leisurely cruise, off-screen flips)\n",
               fish_count, num_frames, fps);
    }

    /* 1. Initialize Fish Agents */
    FishAgent* agents = (FishAgent*)malloc(fish_count * sizeof(FishAgent));
    if (!agents) return -2;

    const char* default_themes[] = { "sapphire", "flame", "emerald", "cyan" };

    /* Parse user seeds, themes, commits, and languages if provided */
    char parsed_seeds[8][64];
    char parsed_themes[8][64];
    char parsed_commits[8][64];
    char parsed_langs[8][128];
    int parsed_seeds_cnt   = split_comma_string(args->fish_seeds, parsed_seeds, 8);
    int parsed_themes_cnt  = split_comma_string(args->fish_themes, parsed_themes, 8);
    int parsed_commits_cnt = split_comma_string(args->fish_commits, parsed_commits, 8);
    int parsed_langs_cnt   = split_delimiter_string(args->fish_languages, ';', parsed_langs, 8);

    for (int k = 0; k < fish_count; k++) {
        FishAgent* fa = &agents[k];
        if (k < parsed_seeds_cnt) {
            snprintf(fa->seed, sizeof(fa->seed), "%.63s", parsed_seeds[k]);
        } else {
            const char* base = args->repo ? args->repo : (args->seed ? args->seed : "betta");
            snprintf(fa->seed, sizeof(fa->seed), "%s-%d", base, k + 1);
        }

        if (k < parsed_themes_cnt) {
            snprintf(fa->theme, sizeof(fa->theme), "%.31s", parsed_themes[k]);
        } else {
            snprintf(fa->theme, sizeof(fa->theme), "%s", default_themes[k % 4]);
        }

        fa->commits = 0;
        fa->languages[0] = '\0';
        if (k < parsed_commits_cnt) {
            fa->commits = atoi(parsed_commits[k]);
        } else if (args->commits > 0) {
            fa->commits = args->commits;
        }

        if (k < parsed_langs_cnt) {
            snprintf(fa->languages, sizeof(fa->languages), "%.127s", parsed_langs[k]);
        } else if (args->languages) {
            snprintf(fa->languages, sizeof(fa->languages), "%.127s", args->languages);
        }

        fa->is_repo = (fa->commits > 0 || fa->languages[0] != '\0' || args->repo != NULL);
        if (fa->is_repo) {
            int c = (fa->commits > 0) ? fa->commits : 50;
            fa->repo_profile = repo_profile_parse(fa->seed, c, fa->languages);
        }

        fa->traits = betta_traits_from_seed(fa->seed);
        if (fa->is_repo) {
            betta_grow(&fa->traits, fa->repo_profile.commits);
        }

        if (is_stream_mode) {
            /* Option B: Terraced streaming stations across the aquarium (peaceful upstream hover) */
            float station_x_ratios[6] = { 0.48f, 0.74f, 0.30f, 0.58f, 0.82f, 0.35f };
            float station_y_ratios[6] = { 0.46f, 0.28f, 0.65f, 0.76f, 0.52f, 0.24f };
            int idx = k % 6;

            fa->x0 = (float)tank_w * station_x_ratios[idx];
            fa->y0 = (float)tank_h * station_y_ratios[idx];

            /* Serene micro-surge and gentle buoyancy drift */
            fa->rx = 6.0f + (float)(k * 3 % 5); /* 6 to 10 px subtle drift */
            fa->ry = 3.0f + (float)(k * 2 % 3); /* 3 to 5 px vertical buoyancy */
            fa->freq_x = 1;
            fa->freq_y = 1;

            float angle_offset = 2.0f * (float)M_PI * (float)k / (float)fish_count;
            fa->phase_x = angle_offset;
            fa->phase_y = angle_offset + 0.5f * (float)M_PI;
            fa->phase_z = angle_offset + 0.8f;

            fa->beats_per_loop = 2; /* 2 full slow fluid swim cycles per loop */
            fa->beat_offset = (float)k * 2.1f;
            fa->scale_factor = 0.92f + 0.08f * (float)(k % 2);
        } else {
            /* Cross Mode: Ultra-slow leisurely traverse with tight off-screen turnaround */
            float offscreen_margin = 54.0f;
            fa->rx = (float)tank_w * 0.5f + offscreen_margin;

            /* Stagger vertical lanes to prevent overlap */
            float y_min = 44.0f;
            float y_max = (float)tank_h - 44.0f;
            float y_step = (fish_count > 1) ? (y_max - y_min) / (float)(fish_count - 1) : 0.0f;
            fa->y0 = y_min + (float)k * y_step;

            /* Gentle vertical buoyancy oscillation during horizontal cross */
            fa->ry = 2.0f + (float)(k % 3) * 0.6f;

            float angle_offset = 2.0f * (float)M_PI * (float)k / (float)fish_count;
            fa->phase_x = angle_offset;
            fa->phase_y = angle_offset * 1.5f;
            fa->phase_z = -0.4f + 0.8f * ((float)k / (float)(fish_count > 1 ? fish_count - 1 : 1));

            /* Chill silk undulation: ~34-38 frames per swim beat */
            int beats = (int)roundf((float)num_frames / 36.0f);
            if (beats < 1) beats = 1;
            fa->beats_per_loop = beats;
            fa->beat_offset = (float)k * 2.1f;
            fa->scale_factor = 0.92f + 0.08f * (float)(k % 3) / 2.0f;
        }

        if (fa->is_repo) {
            printf("  Fish #%d: Repo \"%s\" [%d commits - %s] | %s | %s\n",
                   k + 1, fa->repo_profile.repo_name, fa->repo_profile.commits,
                   repo_growth_stage_name(repo_growth_stage(fa->repo_profile.commits)),
                   fin_style_name(fa->traits.fin_style),
                   body_archetype_name(fa->traits.body_archetype));
        } else {
            printf("  Fish #%d: Seed \"%s\" (%s) | %s (%s) | %s (%s)\n",
                   k + 1, fa->seed, fa->theme,
                   fin_style_name(fa->traits.fin_style),
                   fin_pattern_name(fa->traits.fin_pattern),
                   body_archetype_name(fa->traits.body_archetype),
                   body_pattern_name(fa->traits.body_pattern));
        }
    }

    /* 2. Initialize Ambient Air Bubbles */
    AmbientBubble bubbles[18];
    for (int b = 0; b < 18; b++) {
        bubbles[b].x = 12.0f + (float)(b * 19 % (tank_w - 24));
        bubbles[b].y_start = (float)((b * 37) % tank_h);
        bubbles[b].speed = 7.0f + (float)(b % 3) * 2.5f;
        bubbles[b].size = (b % 3 == 0) ? 2 : 1;
    }

    /* 3. Prepare Frame Buffers */
    Color* tank_canvas = (Color*)malloc(tank_w * tank_h * sizeof(Color));
    int out_w = tank_w * scale;
    int out_h = tank_h * scale;
    uint8_t* out_rgba = (uint8_t*)malloc(out_w * out_h * 4);

    if (!tank_canvas || !out_rgba) {
        free(agents);
        if (tank_canvas) free(tank_canvas);
        if (out_rgba) free(out_rgba);
        return -3;
    }

    GifStream* stream = gif_stream_open(out_gif, out_w, out_h, delay_cs);
    if (!stream) {
        free(agents);
        free(tank_canvas);
        free(out_rgba);
        fprintf(stderr, "Error opening GIF output: %s\n", out_gif);
        return -4;
    }

    /* Background tank color: deep ocean midnight blue */
    Color bg_color = { 10, 14, 24, 255 };

    /* 4. Render Animation Frames */
    for (int frame = 0; frame < num_frames; frame++) {
        float tau = 2.0f * (float)M_PI * (float)frame / (float)num_frames;

        /* Clear Tank Canvas */
        for (int i = 0; i < tank_w * tank_h; i++) {
            tank_canvas[i] = bg_color;
        }

        /* Ambient subtle vertical water gradient */
        for (int y = 0; y < tank_h; y++) {
            float gy = (float)y / (float)tank_h;
            Color line_col;
            line_col.r = (uint8_t)(8.0f + 6.0f * (1.0f - gy));
            line_col.g = (uint8_t)(12.0f + 14.0f * (1.0f - gy));
            line_col.b = (uint8_t)(22.0f + 20.0f * (1.0f - gy));
            line_col.a = 255;
            for (int x = 0; x < tank_w; x++) {
                tank_canvas[y * tank_w + x] = line_col;
            }
        }

        /* Ambient Water Bubbles (peaceful, tranquil, seamless loop) */
        for (int b = 0; b < 18; b++) {
            float by, bx_f;
            if (is_stream_mode) {
                /* Calm stream: gentle upward drift and subtle horizontal current sway */
                by = bubbles[b].y_start - 6.0f * sinf(tau + (float)b * 0.8f);
                bx_f = bubbles[b].x - 3.0f * sinf(tau + (float)b * 0.5f);
            } else {
                /* Serene buoyant floating micro-bubbles in open tank */
                by = bubbles[b].y_start - 5.0f * sinf(tau + (float)b * 0.7f);
                bx_f = bubbles[b].x + 2.0f * sinf(tau * 2.0f + (float)b);
            }

            while (by < 0.0f) by += (float)tank_h;
            while (by >= (float)tank_h) by -= (float)tank_h;
            while (bx_f < 0.0f) bx_f += (float)tank_w;
            while (bx_f >= (float)tank_w) bx_f -= (float)tank_w;

            int bx = (int)roundf(bx_f);
            int biy = (int)roundf(by);

            if (bx >= 0 && bx < tank_w && biy >= 0 && biy < tank_h) {
                tank_canvas[biy * tank_w + bx] = (Color){ 90, 160, 210, 255 };
                if (bubbles[b].size > 1 && bx + 1 < tank_w && biy + 1 < tank_h) {
                    tank_canvas[biy * tank_w + bx + 1] = (Color){ 160, 210, 240, 255 };
                    tank_canvas[(biy + 1) * tank_w + bx] = (Color){ 60, 120, 180, 255 };
                }
            }
        }

        /* Compute Fish Positions, Headings, and Depths */
        float fish_x[8], fish_y[8], fish_z[8];
        int fish_facing[8];
        float fish_swim_phase[8];
        int sort_order[8];

        for (int k = 0; k < fish_count; k++) {
            sort_order[k] = k;
            FishAgent* fa = &agents[k];

            if (is_stream_mode) {
                /* Option B: Surge-and-glide upstream drift */
                fish_x[k] = fa->x0 + fa->rx * cosf(tau + fa->phase_x);
                fish_y[k] = fa->y0 + fa->ry * sinf(tau + fa->phase_y);
                fish_z[k] = sinf(tau + fa->phase_z);
                fish_facing[k] = 1; /* Always facing upstream (left) */
            } else {
                /* Cross Mode: Steady chill cruise across screen, turnaround strictly off-screen */
                float eps = 0.04f;
                float norm = asinf(1.0f - eps);
                float theta = tau + fa->phase_x;
                float cos_th = cosf(theta);
                float sin_th = sinf(theta);

                float arg = (1.0f - eps) * cos_th;
                if (arg > 0.9999f) arg = 0.9999f;
                if (arg < -0.9999f) arg = -0.9999f;

                float u = asinf(arg) / norm; /* Steady cruise from +1.0 to -1.0 */
                fish_x[k] = (float)tank_w * 0.5f + fa->rx * u;
                fish_y[k] = fa->y0 + fa->ry * sinf(2.0f * tau + fa->phase_y);
                fish_z[k] = fa->phase_z + 0.15f * sinf(tau + fa->phase_y);
                fish_facing[k] = (-sin_th <= 0.0f) ? 1 : 0; /* 1: swimming left, 0: swimming right */
            }

            /* Swim undulation phase */
            fish_swim_phase[k] = (float)fa->beats_per_loop * tau + fa->beat_offset;
        }

        /* Depth Sort (Painter's Algorithm: lowest Z / deepest drawn first) */
        for (int i = 0; i < fish_count - 1; i++) {
            for (int j = i + 1; j < fish_count; j++) {
                if (fish_z[sort_order[i]] > fish_z[sort_order[j]]) {
                    int tmp = sort_order[i];
                    sort_order[i] = sort_order[j];
                    sort_order[j] = tmp;
                }
            }
        }

        /* Render each fish in depth order and blit onto tank */
        for (int s_idx = 0; s_idx < fish_count; s_idx++) {
            int k = sort_order[s_idx];
            FishAgent* fa = &agents[k];

            PixelGrid fgrid;
            if (fa->is_repo) {
                palette_from_repo(&fgrid, &fa->repo_profile);
            } else {
                palette_load_theme(&fgrid, parse_theme_name(fa->theme));
            }
            grid_init(&fgrid, PAL_BG);

            BettaBodyConfig bcfg = betta_body_default_config();
            bcfg.center_x    = 56;
            bcfg.center_y    = (fa->traits.gravity_droop > 0.65f) ? 50 : 54;
            bcfg.length      = (int)roundf((float)fa->traits.body_length * fa->scale_factor);
            bcfg.plumpness   = fa->traits.belly_depth;
            bcfg.swim_phase  = fish_swim_phase[k];
            bcfg.swim_amp    = (is_stream_mode ? 1.4f : 1.6f) * fa->scale_factor; /* Relaxed silky undulation */
            bcfg.facing_left = 1; /* Render standard complete fish into fgrid */
            bcfg.traits      = fa->traits;

            BettaFinConfig fin_cfg = betta_fin_default_config();
            fin_cfg.flow_phase = fish_swim_phase[k];
            fin_cfg.fin_scale  = fa->scale_factor;
            fin_cfg.traits     = fa->traits;

            /* Render full fish into local grid (all fins intact, zero clipping) */
            render_betta_fins_bg(&fgrid, &bcfg, &fin_cfg);
            render_betta_body(&fgrid, &bcfg);
            render_betta_fins_fg(&fgrid, &bcfg, &fin_cfg);

            /* Blit local grid to tank canvas with horizontal flip according to fish_facing[k] */
            int origin_x = (int)roundf(fish_x[k]);
            int origin_y = (int)roundf(fish_y[k]);
            int facing_left = fish_facing[k];

            for (int gy = 0; gy < GRID_HEIGHT; gy++) {
                int ty = origin_y + (gy - bcfg.center_y);
                if (ty < 0 || ty >= tank_h) continue;

                for (int gx = 0; gx < GRID_WIDTH; gx++) {
                    uint8_t pixel_idx = fgrid.pixels[gy][gx];
                    if (pixel_idx == PAL_BG) continue;

                    /* Blit with orientation (1: left, 0: right) */
                    int tx = facing_left ? (origin_x + (gx - bcfg.center_x))
                                         : (origin_x + (bcfg.center_x - gx));
                    if (tx < 0 || tx >= tank_w) continue;

                    Color c = fgrid.palette[pixel_idx];

                    /* Atmospheric depth fog for background fish */
                    if (fish_z[k] < -0.2f) {
                        float fog = -fish_z[k] * 0.35f;
                        c.r = (uint8_t)((1.0f - fog) * (float)c.r + fog * 10.0f);
                        c.g = (uint8_t)((1.0f - fog) * (float)c.g + fog * 16.0f);
                        c.b = (uint8_t)((1.0f - fog) * (float)c.b + fog * 26.0f);
                    }

                    tank_canvas[ty * tank_w + tx] = c;
                }
            }
        }

        /* Upscale tank canvas to output RGBA buffer */
        for (int y = 0; y < out_h; y++) {
            int src_y = y / scale;
            for (int x = 0; x < out_w; x++) {
                int src_x = x / scale;
                Color c = tank_canvas[src_y * tank_w + src_x];
                int idx = (y * out_w + x) * 4;
                out_rgba[idx + 0] = c.r;
                out_rgba[idx + 1] = c.g;
                out_rgba[idx + 2] = c.b;
                out_rgba[idx + 3] = c.a;
            }
        }

        /* Write GIF frame */
        gif_stream_write_frame(stream, out_rgba);
    }

    gif_stream_close(stream);

    free(agents);
    free(tank_canvas);
    free(out_rgba);

    printf("Successfully saved Aquarium GIF to: %s (%dx%d, %d frames)\n",
           out_gif, out_w, out_h, num_frames);
    return 0;
}
