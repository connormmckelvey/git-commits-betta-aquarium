#include "betta_body.h"
#include "palette.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BettaBodyConfig betta_body_default_config(void) {
    BettaBodyConfig cfg;
    cfg.center_x    = 56;
    cfg.center_y    = 50;
    cfg.length      = 64;
    cfg.plumpness   = 1.0f;
    cfg.mouth_open  = 0;
    cfg.eye_blink   = 0;
    cfg.gill_flare  = 0;
    cfg.swim_phase  = 0.0f;
    cfg.swim_amp    = 2.8f;
    cfg.facing_left = 1;
    cfg.traits      = betta_traits_from_seed("git-betta");
    return cfg;
}

static float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static inline void draw_pix(PixelGrid* g, int x, int y, uint8_t color, const BettaBodyConfig* cfg) {
    (void)cfg;
    grid_set_pixel(g, x, y, color);
}

int betta_get_spine_y(const BettaBodyConfig* bcfg, int x) {
    int len = (bcfg->length > 0) ? bcfg->length : 64;
    int x_start = bcfg->center_x - len / 2;
    float u = (float)(x - x_start) / (float)len;
    u = clampf(u, 0.0f, 1.0f);

    /* Resting posture arch */
    float arch = (bcfg->traits.body_archetype == BODY_ARCHETYPE_ROYAL) ? 1.6f :
                 (bcfg->traits.body_archetype == BODY_ARCHETYPE_SLEEK) ? 0.7f : 1.2f;
    float resting_y = (float)bcfg->center_y - arch * sinf((float)M_PI * u);

    /* Dynamic kinematic undulation wave */
    if (bcfg->swim_amp > 0.001f) {
        float amp = bcfg->swim_amp * (0.12f + 0.88f * u * u);
        float wave = amp * sinf(bcfg->swim_phase - 2.0f * (float)M_PI * 0.85f * u);
        resting_y += wave;
    }

    return (int)roundf(resting_y);
}

int betta_get_body_top_y(const BettaBodyConfig* bcfg, int x) {
    int len = (bcfg->length > 0) ? bcfg->length : 64;
    int x_start = bcfg->center_x - len / 2;
    float u = (float)(x - x_start) / (float)len;
    u = clampf(u, 0.0f, 1.0f);

    int y_spine = betta_get_spine_y(bcfg, x);
    float h_crown = (bcfg->traits.crown_height > 8.0f) ? bcfg->traits.crown_height : 13.0f;
    float peduncle = (bcfg->traits.peduncle_depth > 1.0f) ? bcfg->traits.peduncle_depth : 3.2f;

    float h_top_f;
    if (u < 0.28f) {
        float t = u / 0.28f;
        float snout_factor = (bcfg->traits.snout_style == SNOUT_ARROW) ? 1.5f :
                             (bcfg->traits.snout_style == SNOUT_ROMAN) ? 2.5f : 2.0f;
        h_top_f = snout_factor + (h_crown - snout_factor) * sinf(0.5f * (float)M_PI * t);
    } else {
        float t = (u - 0.28f) / 0.72f;
        float cos_val = cosf(0.5f * (float)M_PI * t);
        if (cos_val < 0.0f) cos_val = 0.0f;
        float pwr = (bcfg->traits.body_archetype == BODY_ARCHETYPE_GIANT)  ? 0.92f :
                    (bcfg->traits.body_archetype == BODY_ARCHETYPE_DRAGON) ? 0.85f : 0.78f;
        h_top_f = peduncle + (h_crown - peduncle) * powf(cos_val, pwr);
    }

    return y_spine - (int)roundf(h_top_f);
}

int betta_get_body_bot_y(const BettaBodyConfig* bcfg, int x) {
    int len = (bcfg->length > 0) ? bcfg->length : 64;
    int x_start = bcfg->center_x - len / 2;
    float u = (float)(x - x_start) / (float)len;
    u = clampf(u, 0.0f, 1.0f);

    int y_spine = betta_get_spine_y(bcfg, x);
    float peduncle = (bcfg->traits.peduncle_depth > 1.0f) ? bcfg->traits.peduncle_depth : 3.2f;
    float base_depth = (bcfg->traits.body_archetype == BODY_ARCHETYPE_GIANT)   ? 15.2f :
                       (bcfg->traits.body_archetype == BODY_ARCHETYPE_COMPACT) ? 13.5f :
                       (bcfg->traits.body_archetype == BODY_ARCHETYPE_SLEEK)   ? 9.2f :
                       (bcfg->traits.body_archetype == BODY_ARCHETYPE_FIGHTER) ? 10.0f :
                       (bcfg->traits.body_archetype == BODY_ARCHETYPE_DRAGON)  ? 11.0f : 12.0f;

    float belly_curve = sinf((float)M_PI * powf(u, 0.74f));
    float h_bot_f = peduncle * (0.8f + 0.2f * u) + (base_depth * bcfg->plumpness) * belly_curve;

    return y_spine + (int)roundf(h_bot_f);
}

void render_betta_body(PixelGrid* g, const BettaBodyConfig* cfg) {
    if (!g || !cfg) return;

    int len = (cfg->length > 0) ? cfg->length : 64;
    int x_start = cfg->center_x - len / 2;
    int x_end   = x_start + len;

    /* Light vector from upper-left-front (swimming left) */
    float lx = -0.38f, ly = -0.72f, lz = 0.58f;
    float l_len = sqrtf(lx * lx + ly * ly + lz * lz);
    lx /= l_len; ly /= l_len; lz /= l_len;

    /* Halfway vector for specular highlights */
    float hx = lx, hy = ly, hz = lz + 1.0f;
    float h_len = sqrtf(hx * hx + hy * hy + hz * hz);
    hx /= h_len; hy /= h_len; hz /= h_len;

    /* --- PASS 1: Volumetric Body Silhouette & 2.5D Surface Shading --- */
    for (int x = x_start; x <= x_end; x++) {
        float u = (float)(x - x_start) / (float)len;
        u = clampf(u, 0.0f, 1.0f);

        int y_spine = betta_get_spine_y(cfg, x);
        int y_top   = betta_get_body_top_y(cfg, x);
        int y_bot   = betta_get_body_bot_y(cfg, x);

        float dx_slope = (u < 0.28f) ? -0.35f * (1.0f - u / 0.28f) : 0.22f * ((u - 0.28f) / 0.72f);

        for (int y = y_top; y <= y_bot; y++) {
            /* 1. Silhouette Outline */
            if (y == y_top || y == y_bot || x == x_start || x == x_end) {
                draw_pix(g, x, y, PAL_OUTLINE, cfg);
                continue;
            }

            /* 2. Compute 3D surface normal */
            float v;
            if (y < y_spine) {
                v = -(float)(y_spine - y) / (float)(y_spine - y_top);
            } else {
                v = (float)(y - y_spine) / (float)(y_bot - y_spine);
            }
            v = clampf(v, -1.0f, 1.0f);

            float z_sq = 1.0f - v * v;
            float z = (z_sq > 0.0f) ? sqrtf(z_sq) : 0.0f;

            float nx = dx_slope;
            float ny = v;
            float nz = z;
            float n_len = sqrtf(nx * nx + ny * ny + nz * nz);
            if (n_len > 0.0001f) {
                nx /= n_len; ny /= n_len; nz /= n_len;
            }

            /* Diffuse & Specular */
            float ndotl = nx * lx + ny * ly + nz * lz;
            float diffuse = (ndotl > 0.0f) ? ndotl : 0.0f;

            float ndoth = nx * hx + ny * hy + nz * hz;
            float spec = (ndoth > 0.0f) ? powf(ndoth, 14.0f) : 0.0f;

            /* Procedural scale pattern & body coloration patterns */
            float scale_mod = 0.0f;
            int scale_size = (cfg->traits.scale_step >= 3) ? cfg->traits.scale_step : 4;
            int su = (x + y / 2) % scale_size;
            int sv = (x - y / 2) % scale_size;
            if (su < 0) su += scale_size;
            if (sv < 0) sv += scale_size;

            if (u > 0.16f && u < 0.88f) {
                if (cfg->traits.body_pattern == BODY_PATTERN_DRAGON_ARMOR || cfg->traits.body_archetype == BODY_ARCHETYPE_DRAGON) {
                    if (su == 0 || sv == 0) {
                        scale_mod += 0.22f; /* Bright metallic scale ridge */
                    } else if (su == scale_size - 1 || sv == scale_size - 1) {
                        scale_mod -= 0.18f; /* Deep scale pocket crease */
                    }
                } else {
                    if (su == 0 || sv == 0) {
                        scale_mod += 0.10f;
                    } else if (su == scale_size - 1 || sv == scale_size - 1) {
                        scale_mod -= 0.07f;
                    }
                }

                /* Procedural Koi Marbling across flank */
                if (cfg->traits.body_pattern == BODY_PATTERN_MARBLE) {
                    float m_wave1 = sinf(u * 14.0f + v * 8.0f + cfg->traits.marble_seed_offset);
                    float m_wave2 = cosf(u * 7.0f - v * 11.0f + cfg->traits.marble_seed_offset * 1.6f);
                    float marble = m_wave1 + 0.6f * m_wave2;
                    if (marble > 0.55f) {
                        scale_mod += 0.26f; /* Light shimmer patch */
                    } else if (marble < -0.45f) {
                        scale_mod -= 0.32f; /* Deep velvety patch */
                    }
                }

                /* Procedural Grizzle (salt-and-pepper micro-stippling) */
                if (cfg->traits.body_pattern == BODY_PATTERN_GRIZZLE) {
                    int h = (x * 7919 + y * 6271) % 100;
                    if (h < 22) scale_mod += 0.18f;
                    else if (h > 78) scale_mod -= 0.18f;
                }
            }

            /* Metallic Full Helmet Mask */
            if (cfg->traits.body_pattern == BODY_PATTERN_MASKED && u < cfg->traits.mask_coverage * 0.30f) {
                scale_mod += 0.35f; /* Solid dragon armor face mask */
            }

            float countershade = -0.15f * v;
            float intensity = 0.18f + 0.58f * diffuse + 0.45f * spec + scale_mod + countershade;

            /* Specular glints along crest */
            if (y <= y_top + 2 && u > 0.12f && u < 0.80f) {
                if (cfg->traits.dorsal_shimmer_mode == 1) {
                    draw_pix(g, x, y, (x % 2 == 0) ? PAL_PEARL_WHITE : PAL_HIGHLIGHT_BRIGHT, cfg);
                    continue;
                } else if (cfg->traits.dorsal_shimmer_mode == 2) {
                    if ((x + 2) % 5 == 0) {
                        draw_pix(g, x, y, PAL_PEARL_WHITE, cfg);
                        continue;
                    }
                } else {
                    if ((x + 2) % 6 == 0) {
                        draw_pix(g, x, y, PAL_PEARL_WHITE, cfg);
                        continue;
                    } else if ((x + 2) % 3 == 0) {
                        draw_pix(g, x, y, PAL_IRIDESCENT_GLINT, cfg);
                        continue;
                    }
                }
            }

            /* Ventral underbelly shadow */
            if (y >= y_bot - 2) {
                draw_pix(g, x, y, (y == y_bot - 1) ? PAL_SHADOW_DEEP : PAL_SHADOW_MID, cfg);
                continue;
            }

            /* Map intensity to palette ramp */
            uint8_t color_idx;
            if (intensity < 0.22f) {
                color_idx = PAL_SHADOW_DEEP;
            } else if (intensity < 0.33f) {
                color_idx = PAL_SHADOW_MID;
            } else if (intensity < 0.45f) {
                color_idx = PAL_BODY_DARK;
            } else if (intensity < 0.58f) {
                color_idx = PAL_BODY_MID;
            } else if (intensity < 0.70f) {
                color_idx = PAL_BODY_LIGHT;
            } else if (intensity < 0.82f) {
                color_idx = PAL_HIGHLIGHT_SOFT;
            } else if (intensity < 0.94f) {
                color_idx = PAL_HIGHLIGHT_BRIGHT;
            } else {
                color_idx = PAL_IRIDESCENT_GLINT;
            }

            draw_pix(g, x, y, color_idx, cfg);
        }
    }

    /* --- PASS 2: Snout & Lips --- */
    int mouth_x = x_start;
    int mouth_y = betta_get_spine_y(cfg, x_start);

    if (cfg->traits.snout_style == SNOUT_ROMAN) {
        draw_pix(g, mouth_x - 1, mouth_y - 2, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y - 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 3, mouth_y,     PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y,     PAL_HIGHLIGHT_BRIGHT, cfg);
        draw_pix(g, mouth_x - 1, mouth_y,     PAL_BODY_LIGHT, cfg);
        draw_pix(g, mouth_x - 3, mouth_y + 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y + 1, cfg->mouth_open ? PAL_SHADOW_DEEP : PAL_BODY_DARK, cfg);
        draw_pix(g, mouth_x - 1, mouth_y + 1, PAL_BODY_MID, cfg);
        draw_pix(g, mouth_x - 2, mouth_y + 2, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 1, mouth_y + 2, PAL_OUTLINE, cfg);
    } else if (cfg->traits.snout_style == SNOUT_ARROW) {
        draw_pix(g, mouth_x - 1, mouth_y - 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y,     PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 3, mouth_y,     PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 1, mouth_y,     cfg->mouth_open ? PAL_SHADOW_DEEP : PAL_HIGHLIGHT_BRIGHT, cfg);
        draw_pix(g, mouth_x - 2, mouth_y + 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 1, mouth_y + 1, PAL_BODY_DARK, cfg);
    } else if (cfg->traits.snout_style == SNOUT_BULLDOG) {
        draw_pix(g, mouth_x - 1, mouth_y - 2, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y - 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 1, mouth_y - 1, PAL_HIGHLIGHT_BRIGHT, cfg);
        draw_pix(g, mouth_x - 3, mouth_y,     PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y,     cfg->mouth_open ? PAL_SHADOW_DEEP : PAL_BODY_DARK, cfg);
        draw_pix(g, mouth_x - 1, mouth_y,     PAL_BODY_MID, cfg);
        /* Muscular fighter underbite jaw */
        draw_pix(g, mouth_x - 4, mouth_y + 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 3, mouth_y + 1, PAL_HIGHLIGHT_BRIGHT, cfg);
        draw_pix(g, mouth_x - 2, mouth_y + 1, PAL_BODY_LIGHT, cfg);
        draw_pix(g, mouth_x - 3, mouth_y + 2, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y + 2, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 1, mouth_y + 2, PAL_OUTLINE, cfg);
    } else {
        draw_pix(g, mouth_x - 1, mouth_y - 2, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 2, mouth_y - 1, PAL_OUTLINE, cfg);
        draw_pix(g, mouth_x - 1, mouth_y - 1, PAL_HIGHLIGHT_BRIGHT, cfg);
        draw_pix(g, mouth_x,     mouth_y - 1, PAL_BODY_LIGHT, cfg);

        if (cfg->mouth_open) {
            draw_pix(g, mouth_x - 1, mouth_y,     PAL_OUTLINE, cfg);
            draw_pix(g, mouth_x,     mouth_y,     PAL_SHADOW_DEEP, cfg);
            draw_pix(g, mouth_x - 1, mouth_y + 1, PAL_OUTLINE, cfg);
            draw_pix(g, mouth_x,     mouth_y + 1, PAL_BODY_MID, cfg);
            draw_pix(g, mouth_x - 1, mouth_y + 2, PAL_OUTLINE, cfg);
        } else {
            draw_pix(g, mouth_x - 1, mouth_y,     PAL_OUTLINE, cfg);
            draw_pix(g, mouth_x,     mouth_y,     PAL_OUTLINE, cfg);
            draw_pix(g, mouth_x - 1, mouth_y + 1, PAL_BODY_DARK, cfg);
            draw_pix(g, mouth_x - 2, mouth_y + 1, PAL_OUTLINE, cfg);
            draw_pix(g, mouth_x - 1, mouth_y + 2, PAL_OUTLINE, cfg);
        }
    }

    /* --- PASS 3: Operculum (Gill Cover Plate) with subtle breathing motion --- */
    int gill_x = x_start + (int)roundf(len * 0.34f);
    int flare_shift = cfg->gill_flare ? -2 : 0;
    int gill_h = (cfg->traits.body_archetype == BODY_ARCHETYPE_COMPACT) ? 7 : 9;

    for (int dy = -gill_h; dy <= gill_h; dy++) {
        float f_dy = (float)(dy + gill_h) / (float)(gill_h * 2);
        int curve_offset;
        if (cfg->traits.gill_curve_style == 1) {
            curve_offset = (int)roundf(2.6f * sinf(0.5f * (float)M_PI * f_dy) + 0.8f * sinf(4.0f * (float)M_PI * f_dy));
        } else if (cfg->traits.gill_curve_style == 2) {
            curve_offset = (int)roundf(3.2f * powf(f_dy, 0.7f));
        } else {
            curve_offset = (int)roundf(2.4f * sinf(0.5f * (float)M_PI * f_dy));
        }

        int gx = gill_x + flare_shift + curve_offset;
        int gy = betta_get_spine_y(cfg, gx) + dy;

        uint8_t current = grid_get_pixel(g, cfg->facing_left ? gx : (2 * cfg->center_x - gx), gy);
        if (current != PAL_OUTLINE && current != PAL_BG) {
            draw_pix(g, gx, gy, cfg->gill_flare ? PAL_HIGHLIGHT_BRIGHT : PAL_GILL_CREASE, cfg);
            draw_pix(g, gx - 1, gy, cfg->gill_flare ? PAL_PEARL_WHITE : PAL_HIGHLIGHT_SOFT, cfg);
        }
    }

    /* --- PASS 4: High-Res Expressive Eye --- */
    int eye_x = x_start + (int)roundf(len * 0.17f);
    int eye_y = betta_get_spine_y(cfg, eye_x) - 4;

    if (cfg->eye_blink) {
        for (int dx = -2; dx <= 2; dx++) {
            draw_pix(g, eye_x + dx, eye_y - 1, PAL_OUTLINE, cfg);
            draw_pix(g, eye_x + dx, eye_y,     PAL_SHADOW_DEEP, cfg);
            draw_pix(g, eye_x + dx, eye_y + 1, PAL_OUTLINE, cfg);
        }
    } else if (cfg->traits.eye_style == EYE_STYLE_SLEEK) {
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                float dist_sq = (float)(dx * dx * 1.0f + dy * dy * 2.2f);
                if (dist_sq <= 8.5f) {
                    draw_pix(g, eye_x + dx, eye_y + dy, PAL_EYE_SOCKET, cfg);
                }
                if (dist_sq <= 4.2f) {
                    draw_pix(g, eye_x + dx, eye_y + dy, PAL_EYE_IRIS, cfg);
                }
            }
        }
        draw_pix(g, eye_x,     eye_y, PAL_EYE_PUPIL, cfg);
        draw_pix(g, eye_x - 1, eye_y, PAL_EYE_PUPIL, cfg);
        draw_pix(g, eye_x + 1, eye_y - 1, PAL_EYE_GLINT, cfg);
        draw_pix(g, eye_x,     eye_y - 1, PAL_PEARL_WHITE, cfg);
    } else {
        float r_socket = (cfg->traits.eye_style == EYE_STYLE_WIDE) ? 12.0f : 9.5f;
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                float dist_sq = (float)(dx * dx + dy * dy);
                if (dist_sq <= r_socket) {
                    draw_pix(g, eye_x + dx, eye_y + dy, PAL_EYE_SOCKET, cfg);
                }
            }
        }

        float r_iris = (cfg->traits.eye_style == EYE_STYLE_WIDE) ? 7.2f : 5.5f;
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                float dist_sq = (float)(dx * dx + dy * dy);
                if (dist_sq <= r_iris) {
                    draw_pix(g, eye_x + dx, eye_y + dy, PAL_EYE_IRIS, cfg);
                }
            }
        }

        draw_pix(g, eye_x - 1, eye_y,     PAL_EYE_PUPIL, cfg);
        draw_pix(g, eye_x,     eye_y,     PAL_EYE_PUPIL, cfg);
        draw_pix(g, eye_x - 1, eye_y + 1, PAL_EYE_PUPIL, cfg);
        draw_pix(g, eye_x,     eye_y + 1, PAL_EYE_PUPIL, cfg);

        draw_pix(g, eye_x + 1, eye_y - 1, PAL_EYE_GLINT, cfg);
        draw_pix(g, eye_x,     eye_y - 1, PAL_EYE_GLINT, cfg);
        draw_pix(g, eye_x + 1, eye_y,     PAL_PEARL_WHITE, cfg);
        if (cfg->traits.eye_style == EYE_STYLE_WIDE) {
            draw_pix(g, eye_x - 1, eye_y - 1, PAL_PEARL_WHITE, cfg);
        }
        draw_pix(g, eye_x - 1, eye_y + 1, PAL_HIGHLIGHT_SOFT, cfg);
    }
}
