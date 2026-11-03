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
    int         stars;
    int         is_recent;
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

static inline void blend_pixel(Color* canvas, int x, int y, int w, int h, Color src) {
    if (x < 0 || x >= w || y < 0 || y >= h) return;
    int idx = y * w + x;
    if (src.a == 255) {
        canvas[idx] = src;
        return;
    }
    if (src.a == 0) return;
    float a = (float)src.a / 255.0f;
    float inv_a = 1.0f - a;
    Color dst = canvas[idx];
    canvas[idx].r = (uint8_t)(src.r * a + dst.r * inv_a);
    canvas[idx].g = (uint8_t)(src.g * a + dst.g * inv_a);
    canvas[idx].b = (uint8_t)(src.b * a + dst.b * inv_a);
    canvas[idx].a = 255;
}

/* Shimmering surface water caustics */
static void render_caustics(Color* canvas, int tank_w, int tank_h, float tau, int is_light_mode) {
    int max_y = 26;
    for (int y = 0; y < max_y && y < tank_h; y++) {
        float depth_fade = 1.0f - (float)y / (float)max_y;
        float y_term = (float)y * 0.18f;
        for (int x = 0; x < tank_w; x++) {
            float x_f = (float)x * 0.055f;
            float wave = sinf(x_f + tau * 2.0f + y_term) * cosf(x_f * 0.6f - tau + y_term * 0.5f);
            if (wave > 0.45f) {
                float intensity = (wave - 0.45f) / 0.55f * depth_fade;
                Color caustic_col;
                if (is_light_mode) {
                    caustic_col = (Color){ 255, 255, 255, (uint8_t)(intensity * 110.0f) };
                } else {
                    caustic_col = (Color){ 100, 200, 255, (uint8_t)(intensity * 75.0f) };
                }
                blend_pixel(canvas, x, y, tank_w, tank_h, caustic_col);
            }
        }
    }
}

/* Textured riverbed sand & pebble substrate along tank bottom */
static void render_substrate(Color* canvas, int tank_w, int tank_h, int is_light_mode) {
    for (int x = 0; x < tank_w; x++) {
        float x_f = (float)x;
        int bed_depth = 12 + (int)roundf(3.5f * sinf(x_f * 0.035f) + 2.0f * cosf(x_f * 0.08f + 1.2f));
        int bed_y = tank_h - bed_depth;
        if (bed_y < 0) bed_y = 0;

        for (int y = bed_y; y < tank_h; y++) {
            float depth = (float)(y - bed_y) / (float)(tank_h - bed_y);
            Color c;
            if (is_light_mode) {
                if (y == bed_y) {
                    c = (Color){ 228, 215, 182, 255 };
                } else if ((x * 17 + y * 31) % 19 == 0) {
                    c = (Color){ 250, 242, 220, 255 };
                } else {
                    uint8_t r = (uint8_t)(205.0f - depth * 35.0f);
                    uint8_t g = (uint8_t)(185.0f - depth * 40.0f);
                    uint8_t b = (uint8_t)(148.0f - depth * 35.0f);
                    c = (Color){ r, g, b, 255 };
                }
            } else {
                if (y == bed_y) {
                    c = (Color){ 36, 44, 56, 255 };
                } else if ((x * 17 + y * 31) % 23 == 0) {
                    c = (Color){ 62, 75, 95, 255 };
                } else {
                    uint8_t r = (uint8_t)(18.0f - depth * 8.0f);
                    uint8_t g = (uint8_t)(24.0f - depth * 10.0f);
                    uint8_t b = (uint8_t)(34.0f - depth * 12.0f);
                    c = (Color){ r, g, b, 255 };
                }
            }
            canvas[y * tank_w + x] = c;
        }
    }

    /* Scattered smooth river pebbles */
    int pebble_xs[8] = { 35, 78, 122, 168, 205, 242, 285, 312 };
    int pebble_rs[8] = { 3, 2, 4, 3, 2, 4, 3, 2 };
    for (int p = 0; p < 8; p++) {
        int px = pebble_xs[p];
        if (px >= tank_w) continue;
        int bed_depth = 12 + (int)roundf(3.5f * sinf((float)px * 0.035f) + 2.0f * cosf((float)px * 0.08f + 1.2f));
        int py = tank_h - bed_depth + 2;
        int pr = pebble_rs[p];

        for (int dy = -pr + 1; dy <= pr; dy++) {
            for (int dx = -pr * 2; dx <= pr * 2; dx++) {
                if ((dx * dx) / (pr * pr * 4.0f) + (dy * dy) / (pr * pr * 1.0f) <= 1.0f) {
                    int tx = px + dx;
                    int ty = py + dy;
                    if (tx >= 0 && tx < tank_w && ty >= 0 && ty < tank_h) {
                        Color pc;
                        if (is_light_mode) {
                            pc = (dy < 0) ? (Color){ 175, 160, 138, 255 } : (Color){ 135, 122, 105, 255 };
                        } else {
                            pc = (dy < 0) ? (Color){ 44, 52, 66, 255 } : (Color){ 24, 30, 40, 255 };
                        }
                        canvas[ty * tank_w + tx] = pc;
                    }
                }
            }
        }
    }
}

/* Swaying procedural kelp stalk */
static void render_kelp_stalk(Color* canvas, int tank_w, int tank_h,
                              float root_x, float root_y, float stalk_h,
                              float max_sway, float stalk_phase, float tau,
                              Color stalk_col, Color rib_col, int width) {
    int steps = (int)stalk_h;
    if (steps <= 0) return;

    for (int s = 0; s < steps; s++) {
        float norm_s = (float)s / stalk_h;
        float sway = sinf(tau + stalk_phase + norm_s * 1.6f) * (norm_s * norm_s * max_sway);
        float cur_x = root_x + sway;
        float cur_y = root_y - (float)s;

        int iy = (int)roundf(cur_y);
        int ix = (int)roundf(cur_x);

        int half_w = width / 2;
        for (int w = -half_w; w <= half_w; w++) {
            int tx = ix + w;
            if (tx >= 0 && tx < tank_w && iy >= 0 && iy < tank_h) {
                Color c = (w == 0 && width > 1) ? rib_col : stalk_col;
                blend_pixel(canvas, tx, iy, tank_w, tank_h, c);
            }
        }
    }
}

/* Background & Foreground Plant Layers */
static void render_aquatic_plants(Color* canvas, int tank_w, int tank_h, float tau, int foreground, int is_light_mode) {
    if (foreground == 0) {
        /* Background tall kelp stalks (drawn behind fish) */
        struct { float x_ratio; float h; float sway; float phase; int w; } bg_stalks[] = {
            { 0.06f, 130.0f, 15.0f, 0.0f, 3 },
            { 0.12f, 145.0f, 18.0f, 1.1f, 3 },
            { 0.28f, 105.0f, 12.0f, 2.3f, 2 },
            { 0.44f, 115.0f, 14.0f, 3.7f, 2 },
            { 0.62f, 125.0f, 16.0f, 0.8f, 3 },
            { 0.78f, 140.0f, 17.0f, 2.4f, 3 },
            { 0.92f, 135.0f, 16.0f, 4.1f, 3 },
            { 0.96f, 150.0f, 19.0f, 5.2f, 3 }
        };
        int num_bg = sizeof(bg_stalks) / sizeof(bg_stalks[0]);

        Color stalk_col, rib_col;
        if (is_light_mode) {
            stalk_col = (Color){ 38, 128, 75, 230 };
            rib_col   = (Color){ 58, 155, 95, 240 };
        } else {
            stalk_col = (Color){ 12, 45, 34, 220 };
            rib_col   = (Color){ 20, 68, 52, 235 };
        }

        for (int i = 0; i < num_bg; i++) {
            float rx = (float)tank_w * bg_stalks[i].x_ratio;
            float ry = (float)tank_h - 10.0f;
            render_kelp_stalk(canvas, tank_w, tank_h, rx, ry,
                              bg_stalks[i].h, bg_stalks[i].sway,
                              bg_stalks[i].phase, tau,
                              stalk_col, rib_col, bg_stalks[i].w);
        }
    } else {
        /* Foreground dwarf grass & hairgrass tufts (drawn over substrate and fish for 3D depth) */
        struct { float x_ratio; float h; float sway; float phase; } fg_tufts[] = {
            { 0.04f, 35.0f, 7.0f, 0.3f },
            { 0.08f, 48.0f, 9.0f, 1.4f },
            { 0.16f, 28.0f, 6.0f, 2.9f },
            { 0.24f, 38.0f, 8.0f, 0.7f },
            { 0.38f, 42.0f, 8.5f, 3.2f },
            { 0.52f, 32.0f, 6.5f, 1.8f },
            { 0.68f, 46.0f, 9.5f, 4.0f },
            { 0.74f, 30.0f, 6.0f, 2.1f },
            { 0.86f, 40.0f, 8.0f, 0.5f },
            { 0.94f, 52.0f, 10.0f, 3.6f }
        };
        int num_fg = sizeof(fg_tufts) / sizeof(fg_tufts[0]);

        Color stalk_col, rib_col;
        if (is_light_mode) {
            stalk_col = (Color){ 52, 165, 90, 245 };
            rib_col   = (Color){ 80, 195, 120, 255 };
        } else {
            stalk_col = (Color){ 22, 85, 58, 240 };
            rib_col   = (Color){ 36, 125, 88, 255 };
        }

        for (int i = 0; i < num_fg; i++) {
            float rx = (float)tank_w * fg_tufts[i].x_ratio;
            float ry = (float)tank_h - 6.0f;
            render_kelp_stalk(canvas, tank_w, tank_h, rx, ry,
                              fg_tufts[i].h, fg_tufts[i].sway,
                              fg_tufts[i].phase, tau,
                              stalk_col, rib_col, 2);
        }
    }
}

/* Star Shimmer Aura: repos with stars emit diamond sparkle glints on their scales */
static void render_star_sparkles(Color* canvas, int tank_w, int tank_h,
                                 int origin_x, int origin_y, int facing_left,
                                 int stars, float tau, int k) {
    if (stars <= 0) return;
    int num_glints = (stars >= 3) ? 3 : stars;

    int flank_dx[3] = { -4, 8, -14 };
    int flank_dy[3] = { -2, 1, -5 };

    for (int g = 0; g < num_glints; g++) {
        float twinkle = sinf(3.0f * tau + (float)k * 1.9f + (float)g * 2.1f);
        if (twinkle < 0.40f) continue;

        int sx = origin_x + (facing_left ? flank_dx[g] : -flank_dx[g]);
        int sy = origin_y + flank_dy[g];

        float bright = (twinkle - 0.40f) / 0.60f;
        Color core = (Color){ 255, 255, 255, (uint8_t)(bright * 255.0f) };
        Color edge = (Color){ 200, 245, 255, (uint8_t)(bright * 180.0f) };

        blend_pixel(canvas, sx, sy, tank_w, tank_h, core);
        blend_pixel(canvas, sx - 1, sy, tank_w, tank_h, edge);
        blend_pixel(canvas, sx + 1, sy, tank_w, tank_h, edge);
        blend_pixel(canvas, sx, sy - 1, tank_w, tank_h, edge);
        blend_pixel(canvas, sx, sy + 1, tank_w, tank_h, edge);

        if (bright > 0.85f) {
            blend_pixel(canvas, sx - 2, sy, tank_w, tank_h, (Color){ 210, 245, 255, 120 });
            blend_pixel(canvas, sx + 2, sy, tank_w, tank_h, (Color){ 210, 245, 255, 120 });
            blend_pixel(canvas, sx, sy - 2, tank_w, tank_h, (Color){ 210, 245, 255, 120 });
            blend_pixel(canvas, sx, sy + 2, tank_w, tank_h, (Color){ 210, 245, 255, 120 });
        }
    }
}

/* Activity Glow: repos committed within 48h emit fresh micro-bubble puffs */
static void render_recent_bubble_puff(Color* canvas, int tank_w, int tank_h,
                                      int origin_x, int origin_y, int facing_left,
                                      float tau, int k) {
    int mouth_x = origin_x + (facing_left ? -24 : 24);
    int mouth_y = origin_y - 2;

    float puff_t = fmodf(tau * 2.0f + (float)k * 1.57f, (float)M_PI);
    float norm_t = puff_t / (float)M_PI;

    float by = (float)mouth_y - norm_t * 22.0f;
    float bx = (float)mouth_x + sinf(norm_t * 6.0f) * 2.5f + (facing_left ? -norm_t * 4.0f : norm_t * 4.0f);

    int ibx = (int)roundf(bx);
    int iby = (int)roundf(by);

    float alpha = (1.0f - norm_t * 0.7f);
    if (alpha < 0.0f) alpha = 0.0f;

    Color bubble_col = (Color){ 210, 245, 255, (uint8_t)(alpha * 230.0f) };
    blend_pixel(canvas, ibx, iby, tank_w, tank_h, bubble_col);
    if (norm_t > 0.2f && norm_t < 0.8f) {
        blend_pixel(canvas, ibx + 1, iby, tank_w, tank_h, (Color){ 170, 225, 255, (uint8_t)(alpha * 160.0f) });
        blend_pixel(canvas, ibx, iby - 1, tank_w, tank_h, (Color){ 255, 255, 255, (uint8_t)(alpha * 200.0f) });
    }
}

int generate_aquarium(const GenerateArgs* args) {
    if (!args) return -1;

    int is_stream_mode = (args->aquarium_mode && strcmp(args->aquarium_mode, "stream") == 0);
    int is_light_mode = (args->lighting_theme && (strcmp(args->lighting_theme, "light") == 0 || strcmp(args->lighting_theme, "lagoon") == 0));

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

    printf("Starting Procedural %s Aquarium Simulation (Lighting: %s | Mode: %s)\n",
           is_stream_mode ? "Stream" : "Cross",
           is_light_mode ? "Sunlit Lagoon (Light)" : "Midnight Abyss (Dark)",
           is_stream_mode ? "Chill Station Hover" : "Slow Traverse");
    printf("  Tank Resolution: %dx%d (Output: %dx%d @ scale %d)\n",
           tank_w, tank_h, tank_w * scale, tank_h * scale, scale);
    printf("  Fish Population: %d fish | Frames: %d @ %d fps\n",
           fish_count, num_frames, fps);

    /* 1. Initialize Fish Agents */
    FishAgent* agents = (FishAgent*)malloc(fish_count * sizeof(FishAgent));
    if (!agents) return -2;

    const char* default_themes[] = { "sapphire", "flame", "emerald", "cyan" };

    /* Parse user seeds, themes, commits, languages, stars, and recent flags */
    char parsed_seeds[8][64];
    char parsed_themes[8][64];
    char parsed_commits[8][64];
    char parsed_langs[8][128];
    char parsed_stars[8][64];
    char parsed_recent[8][64];
    int parsed_seeds_cnt   = split_comma_string(args->fish_seeds, parsed_seeds, 8);
    int parsed_themes_cnt  = split_comma_string(args->fish_themes, parsed_themes, 8);
    int parsed_commits_cnt = split_comma_string(args->fish_commits, parsed_commits, 8);
    int parsed_langs_cnt   = split_delimiter_string(args->fish_languages, ';', parsed_langs, 8);
    int parsed_stars_cnt   = split_comma_string(args->fish_stars, parsed_stars, 8);
    int parsed_recent_cnt  = split_comma_string(args->fish_recent, parsed_recent, 8);

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

        fa->stars = (k < parsed_stars_cnt) ? atoi(parsed_stars[k]) : 0;
        fa->is_recent = (k < parsed_recent_cnt) ? atoi(parsed_recent[k]) : 0;

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
            /* Terraced streaming stations across the aquarium (peaceful upstream hover) */
            float station_x_ratios[6] = { 0.48f, 0.74f, 0.30f, 0.58f, 0.82f, 0.35f };
            float station_y_ratios[6] = { 0.46f, 0.28f, 0.65f, 0.76f, 0.52f, 0.24f };
            int idx = k % 6;

            fa->x0 = (float)tank_w * station_x_ratios[idx];
            fa->y0 = (float)tank_h * station_y_ratios[idx];

            /* Serene micro-surge and gentle buoyancy drift */
            fa->rx = 6.0f + (float)(k * 3 % 5);
            fa->ry = 3.0f + (float)(k * 2 % 3);
            fa->freq_x = 1;
            fa->freq_y = 1;

            float angle_offset = 2.0f * (float)M_PI * (float)k / (float)fish_count;
            fa->phase_x = angle_offset;
            fa->phase_y = angle_offset + 0.5f * (float)M_PI;
            fa->phase_z = angle_offset + 0.8f;

            fa->beats_per_loop = 2; /* 2 full fluid swim cycles per loop */
            fa->beat_offset = (float)k * 2.1f;
            fa->scale_factor = 0.92f + 0.08f * (float)(k % 2);
        } else {
            /* Cross Mode: Leisurely traverse with tight off-screen turnaround */
            float offscreen_margin = 54.0f;
            fa->rx = (float)tank_w * 0.5f + offscreen_margin;

            float y_min = 44.0f;
            float y_max = (float)tank_h - 44.0f;
            float y_step = (fish_count > 1) ? (y_max - y_min) / (float)(fish_count - 1) : 0.0f;
            fa->y0 = y_min + (float)k * y_step;

            fa->ry = 2.0f + (float)(k % 3) * 0.6f;

            float angle_offset = 2.0f * (float)M_PI * (float)k / (float)fish_count;
            fa->phase_x = angle_offset;
            fa->phase_y = angle_offset * 1.5f;
            fa->phase_z = -0.4f + 0.8f * ((float)k / (float)(fish_count > 1 ? fish_count - 1 : 1));

            int beats = (int)roundf((float)num_frames / 36.0f);
            if (beats < 1) beats = 1;
            fa->beats_per_loop = beats;
            fa->beat_offset = (float)k * 2.1f;
            fa->scale_factor = 0.92f + 0.08f * (float)(k % 3) / 2.0f;
        }

        if (fa->is_repo) {
            printf("  Fish #%d: Repo \"%s\" [%d commits - %s | %d stars | %s] | %s | %s\n",
                   k + 1, fa->repo_profile.repo_name, fa->repo_profile.commits,
                   repo_growth_stage_name(repo_growth_stage(fa->repo_profile.commits)),
                   fa->stars, fa->is_recent ? "Active <48h" : "Idle",
                   fin_style_name(fa->traits.fin_style),
                   body_archetype_name(fa->traits.body_archetype));
        } else {
            printf("  Fish #%d: Seed \"%s\" (%s) | %s | %s\n",
                   k + 1, fa->seed, fa->theme,
                   fin_style_name(fa->traits.fin_style),
                   body_archetype_name(fa->traits.body_archetype));
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

    /* 4. Render Animation Frames */
    for (int frame = 0; frame < num_frames; frame++) {
        float tau = 2.0f * (float)M_PI * (float)frame / (float)num_frames;

        /* Ambient vertical water gradient */
        for (int y = 0; y < tank_h; y++) {
            float gy = (float)y / (float)tank_h;
            Color line_col;
            if (is_light_mode) {
                line_col.r = (uint8_t)(170.0f - 55.0f * gy);
                line_col.g = (uint8_t)(222.0f - 40.0f * gy);
                line_col.b = (uint8_t)(240.0f - 30.0f * gy);
                line_col.a = 255;
            } else {
                line_col.r = (uint8_t)(8.0f + 6.0f * (1.0f - gy));
                line_col.g = (uint8_t)(12.0f + 14.0f * (1.0f - gy));
                line_col.b = (uint8_t)(22.0f + 20.0f * (1.0f - gy));
                line_col.a = 255;
            }
            for (int x = 0; x < tank_w; x++) {
                tank_canvas[y * tank_w + x] = line_col;
            }
        }

        /* Surface light caustics */
        render_caustics(tank_canvas, tank_w, tank_h, tau, is_light_mode);

        /* Ambient Water Bubbles */
        for (int b = 0; b < 18; b++) {
            float by, bx_f;
            if (is_stream_mode) {
                by = bubbles[b].y_start - 6.0f * sinf(tau + (float)b * 0.8f);
                bx_f = bubbles[b].x - 3.0f * sinf(tau + (float)b * 0.5f);
            } else {
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
                Color bubble_col = is_light_mode ? (Color){ 245, 255, 255, 220 } : (Color){ 90, 160, 210, 255 };
                blend_pixel(tank_canvas, bx, biy, tank_w, tank_h, bubble_col);
                if (bubbles[b].size > 1 && bx + 1 < tank_w && biy + 1 < tank_h) {
                    Color b_high = is_light_mode ? (Color){ 255, 255, 255, 240 } : (Color){ 160, 210, 240, 255 };
                    Color b_rim  = is_light_mode ? (Color){ 130, 200, 225, 200 } : (Color){ 60, 120, 180, 255 };
                    blend_pixel(tank_canvas, bx + 1, biy, tank_w, tank_h, b_high);
                    blend_pixel(tank_canvas, bx, biy + 1, tank_w, tank_h, b_rim);
                }
            }
        }

        /* Module 1: Planted Aquascape - Background Kelp Layer (drawn behind fish) */
        render_aquatic_plants(tank_canvas, tank_w, tank_h, tau, 0, is_light_mode);

        /* Compute Fish Positions, Headings, and Depths */
        float fish_x[8], fish_y[8], fish_z[8];
        int fish_facing[8];
        float fish_swim_phase[8];
        int sort_order[8];

        for (int k = 0; k < fish_count; k++) {
            sort_order[k] = k;
            FishAgent* fa = &agents[k];

            if (is_stream_mode) {
                fish_x[k] = fa->x0 + fa->rx * cosf(tau + fa->phase_x);
                fish_y[k] = fa->y0 + fa->ry * sinf(tau + fa->phase_y);
                fish_z[k] = sinf(tau + fa->phase_z);
                fish_facing[k] = 1;
            } else {
                float eps = 0.04f;
                float norm = asinf(1.0f - eps);
                float theta = tau + fa->phase_x;
                float cos_th = cosf(theta);
                float sin_th = sinf(theta);

                float arg = (1.0f - eps) * cos_th;
                if (arg > 0.9999f) arg = 0.9999f;
                if (arg < -0.9999f) arg = -0.9999f;

                float u = asinf(arg) / norm;
                fish_x[k] = (float)tank_w * 0.5f + fa->rx * u;
                fish_y[k] = fa->y0 + fa->ry * sinf(2.0f * tau + fa->phase_y);
                fish_z[k] = fa->phase_z + 0.15f * sinf(tau + fa->phase_y);
                fish_facing[k] = (-sin_th <= 0.0f) ? 1 : 0;
            }

            fish_swim_phase[k] = (float)fa->beats_per_loop * tau + fa->beat_offset;
        }

        /* Depth Sort */
        for (int i = 0; i < fish_count - 1; i++) {
            for (int j = i + 1; j < fish_count; j++) {
                if (fish_z[sort_order[i]] > fish_z[sort_order[j]]) {
                    int tmp = sort_order[i];
                    sort_order[i] = sort_order[j];
                    sort_order[j] = tmp;
                }
            }
        }

        /* Render each fish in depth order */
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

            /* Module 3: Natural periodic eye blink (2 frames per loop, staggered per fish) */
            int blink_frame = (k * 19 + 7) % num_frames;

            BettaBodyConfig bcfg = betta_body_default_config();
            bcfg.center_x    = 56;
            bcfg.center_y    = (fa->traits.gravity_droop > 0.65f) ? 50 : 54;
            bcfg.length      = (int)roundf((float)fa->traits.body_length * fa->scale_factor);
            bcfg.plumpness   = fa->traits.belly_depth;
            bcfg.eye_blink   = (frame == blink_frame || frame == blink_frame + 1) ? 1 : 0;
            bcfg.swim_phase  = fish_swim_phase[k];
            bcfg.swim_amp    = (is_stream_mode ? 1.4f : 1.6f) * fa->scale_factor;
            bcfg.facing_left = 1;
            bcfg.traits      = fa->traits;

            /* Module 3: Rapid delicate pectoral fin fluttering (4x swim tempo) */
            BettaFinConfig fin_cfg = betta_fin_default_config();
            fin_cfg.flow_phase     = fish_swim_phase[k];
            fin_cfg.pectoral_phase = 4.0f * tau + fa->beat_offset * 2.5f;
            fin_cfg.fin_scale      = fa->scale_factor;
            fin_cfg.traits         = fa->traits;

            render_betta_fins_bg(&fgrid, &bcfg, &fin_cfg);
            render_betta_body(&fgrid, &bcfg);
            render_betta_fins_fg(&fgrid, &bcfg, &fin_cfg);

            int origin_x = (int)roundf(fish_x[k]);
            int origin_y = (int)roundf(fish_y[k]);
            int facing_left = fish_facing[k];

            for (int gy = 0; gy < GRID_HEIGHT; gy++) {
                int ty = origin_y + (gy - bcfg.center_y);
                if (ty < 0 || ty >= tank_h) continue;

                for (int gx = 0; gx < GRID_WIDTH; gx++) {
                    uint8_t pixel_idx = fgrid.pixels[gy][gx];
                    if (pixel_idx == PAL_BG) continue;

                    int tx = facing_left ? (origin_x + (gx - bcfg.center_x))
                                         : (origin_x + (bcfg.center_x - gx));
                    if (tx < 0 || tx >= tank_w) continue;

                    Color c = fgrid.palette[pixel_idx];

                    /* Atmospheric depth fog / haze */
                    if (fish_z[k] < -0.2f) {
                        float fog = -fish_z[k] * 0.35f;
                        if (is_light_mode) {
                            c.r = (uint8_t)((1.0f - fog) * (float)c.r + fog * 140.0f);
                            c.g = (uint8_t)((1.0f - fog) * (float)c.g + fog * 195.0f);
                            c.b = (uint8_t)((1.0f - fog) * (float)c.b + fog * 220.0f);
                        } else {
                            c.r = (uint8_t)((1.0f - fog) * (float)c.r + fog * 10.0f);
                            c.g = (uint8_t)((1.0f - fog) * (float)c.g + fog * 16.0f);
                            c.b = (uint8_t)((1.0f - fog) * (float)c.b + fog * 26.0f);
                        }
                    }

                    tank_canvas[ty * tank_w + tx] = c;
                }
            }

            /* Module 2: GitHub Star Glint Sparkles */
            if (fa->stars > 0) {
                render_star_sparkles(tank_canvas, tank_w, tank_h,
                                     origin_x, origin_y, facing_left,
                                     fa->stars, tau, k);
            }

            /* Module 2: Recent Activity Code Micro-Bubble Puff (<48h) */
            if (fa->is_recent) {
                render_recent_bubble_puff(tank_canvas, tank_w, tank_h,
                                          origin_x, origin_y, facing_left,
                                          tau, k);
            }
        }

        /* Module 1: Textured Sand & Pebble Substrate */
        render_substrate(tank_canvas, tank_w, tank_h, is_light_mode);

        /* Module 1: Foreground Hairgrass & Plant Layer (3D Parallax in front of fish/sand) */
        render_aquatic_plants(tank_canvas, tank_w, tank_h, tau, 1, is_light_mode);

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
