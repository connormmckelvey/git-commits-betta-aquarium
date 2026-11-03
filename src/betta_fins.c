#include "betta_fins.h"
#include "palette.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BettaFinConfig betta_fin_default_config(void) {
    BettaFinConfig cfg;
    cfg.flow_phase    = 0.0f;
    cfg.fin_scale     = 1.0f;
    cfg.show_caudal   = 1;
    cfg.show_dorsal   = 1;
    cfg.show_anal     = 1;
    cfg.show_ventrals = 1;
    cfg.show_pectoral = 1;
    cfg.traits        = betta_traits_from_seed("git-betta");
    return cfg;
}

static inline void draw_pix(PixelGrid* g, int x, int y, uint8_t color, const BettaBodyConfig* bcfg) {
    (void)bcfg;
    grid_set_pixel(g, x, y, color);
}

static inline uint8_t get_pix(const PixelGrid* g, int x, int y, const BettaBodyConfig* bcfg) {
    (void)bcfg;
    return grid_get_pixel(g, x, y);
}

/* Resolve procedural fin color based on continuous depth ratio and fin pattern */
static inline uint8_t resolve_fin_color(float depth_ratio, int is_ray_rib, int ix, int iy, const BettaFinConfig* fin_cfg) {
    /* 1. Butterfly Two-Tone Pattern (Base body tone + pearl-white outer band) */
    if (fin_cfg->traits.fin_pattern == FIN_PATTERN_BUTTERFLY) {
        float band = (fin_cfg->traits.butterfly_band > 0.5f) ? fin_cfg->traits.butterfly_band : 0.68f;
        if (depth_ratio > band) {
            if (depth_ratio > band + 0.05f) {
                return ((ix + iy) % 3 == 0) ? PAL_PEARL_WHITE : PAL_IRIDESCENT_GLINT;
            } else {
                return PAL_HIGHLIGHT_SOFT;
            }
        }
    }

    /* 2. Luminous Neon Rim Edge */
    if (fin_cfg->traits.fin_pattern == FIN_PATTERN_EDGED) {
        if (depth_ratio > 0.94f) {
            return ((ix + iy) % 2 == 0) ? PAL_PEARL_WHITE : PAL_IRIDESCENT_GLINT;
        }
    }

    /* 3. Striated Contrasting Rib Rays */
    if (fin_cfg->traits.fin_pattern == FIN_PATTERN_STRIATED) {
        if (is_ray_rib) {
            return (depth_ratio > 0.72f) ? PAL_PEARL_WHITE : (depth_ratio > 0.38f ? PAL_HIGHLIGHT_BRIGHT : PAL_HIGHLIGHT_SOFT);
        } else {
            return (depth_ratio > 0.60f) ? PAL_BODY_DARK : PAL_SHADOW_DEEP;
        }
    }

    /* 4. Marbled Mottling on Fins */
    if (fin_cfg->traits.fin_pattern == FIN_PATTERN_MOTTLED) {
        float m_pat = sinf((float)ix * 0.42f + (float)iy * 0.38f + fin_cfg->traits.marble_seed_offset);
        if (m_pat > 0.42f && depth_ratio > 0.38f) {
            return (depth_ratio > 0.75f) ? PAL_PEARL_WHITE : PAL_HIGHLIGHT_BRIGHT;
        } else if (m_pat < -0.42f) {
            return PAL_SHADOW_DEEP;
        }
    }

    /* Standard Smooth Silk Gradient */
    if (depth_ratio > 0.94f) {
        return ((ix + iy) % 3 == 0) ? PAL_PEARL_WHITE : PAL_IRIDESCENT_GLINT;
    } else if (depth_ratio > 0.80f) {
        return PAL_FIN_HIGHLIGHT;
    } else if (depth_ratio > 0.55f) {
        return is_ray_rib ? PAL_FIN_HIGHLIGHT : PAL_FIN_LIGHT;
    } else if (depth_ratio > 0.28f) {
        return PAL_FIN_MID;
    } else {
        return PAL_FIN_DARK;
    }
}

/* --- BACKGROUND FINS (Layer 1: Behind Body) --- */
void render_betta_fins_bg(PixelGrid* g, const BettaBodyConfig* body_cfg, const BettaFinConfig* fin_cfg) {
    if (!g || !body_cfg || !fin_cfg) return;

    /* 1. Background Pelvic Feeler(s) */
    if (fin_cfg->show_ventrals) {
        int x0 = body_cfg->center_x - 14;
        int y0 = betta_get_spine_y(body_cfg, x0) + 7;
        int num_pts = (int)roundf(fin_cfg->traits.ventral_length * 0.88f);
        if (num_pts < 14) num_pts = 14;

        float curl = fin_cfg->traits.ventral_curl * 1.1f;

        /* Primary background streamer */
        for (int i = 0; i < num_pts; i++) {
            float t = (float)i / (float)num_pts;
            float px, py;
            if (fin_cfg->traits.ventral_style == VENTRAL_CURLED) {
                px = (float)x0 + curl * 1.4f * sinf(t * 2.8f);
                py = (float)y0 + (float)num_pts * 0.78f * t;
            } else {
                px = (float)x0 + curl * t * t + 1.4f * sinf(fin_cfg->flow_phase + t * 4.0f);
                py = (float)y0 + (float)num_pts * 0.85f * t;
            }

            int ix = (int)roundf(px);
            int iy = (int)roundf(py);

            uint8_t color = (t > 0.85f) ? PAL_HIGHLIGHT_SOFT : PAL_BODY_DARK;
            draw_pix(g, ix, iy, color, body_cfg);
            draw_pix(g, ix + 1, iy, PAL_SHADOW_DEEP, body_cfg);
        }

        /* Forked secondary background streamer */
        if (fin_cfg->traits.ventral_fork_ratio > 0.1f) {
            int fork_pts = (int)roundf((float)num_pts * fin_cfg->traits.ventral_fork_ratio);
            for (int i = 0; i < fork_pts; i++) {
                float t = (float)i / (float)fork_pts;
                float px = (float)x0 + (curl * 1.4f + 4.0f) * t * t;
                float py = (float)y0 + (float)fork_pts * 0.82f * t;
                int ix = (int)roundf(px);
                int iy = (int)roundf(py);
                draw_pix(g, ix, iy, PAL_SHADOW_DEEP, body_cfg);
            }
        }
    }

    /* 2. Background Caudal Folds (Deep shadowed folds behind tail fan) */
    if (fin_cfg->show_caudal) {
        int x_base = body_cfg->center_x + body_cfg->length / 2 - 2;
        int y_base = betta_get_spine_y(body_cfg, x_base);

        int count = (fin_cfg->traits.cleft_depth > 0.25f) ? 5 : 4;
        for (int i = -count; i <= count; i++) {
            float angle = (float)i * 0.20f;
            float max_len = 36.0f * fin_cfg->fin_scale;

            for (float r = 4.0f; r < max_len; r += 1.0f) {
                float s = r / max_len;
                float px = (float)x_base + r * cosf(angle) + 4.0f * s * s;
                float py = (float)y_base + r * sinf(angle) + 2.5f * sinf(fin_cfg->flow_phase + s * 3.0f);

                int ix = (int)roundf(px);
                int iy = (int)roundf(py);

                if (get_pix(g, ix, iy, body_cfg) == PAL_BG) {
                    draw_pix(g, ix, iy, PAL_SHADOW_DEEP, body_cfg);
                }
            }
        }
    }
}

/* --- FOREGROUND FINS (Layer 3: Billowing Over/Around Body) --- */
void render_betta_fins_fg(PixelGrid* g, const BettaBodyConfig* body_cfg, const BettaFinConfig* fin_cfg) {
    if (!g || !body_cfg || !fin_cfg) return;

    int len = body_cfg->length;
    int x_start = body_cfg->center_x - len / 2;
    int x_end   = x_start + len;

    /* 1. Dorsal Fin (Continuous sail crest) */
    if (fin_cfg->show_dorsal) {
        int d_xstart = x_start + (int)roundf(len * fin_cfg->traits.dorsal_base_start);
        int d_xend   = x_start + (int)roundf(len * fin_cfg->traits.dorsal_base_end);
        if (d_xend <= d_xstart) d_xend = d_xstart + 1;

        for (int x = d_xstart; x <= d_xend; x++) {
            int y_base = betta_get_body_top_y(body_cfg, x);
            float u = (float)(x - d_xstart) / (float)(d_xend - d_xstart);

            /* Arching crest profile */
            float h_fin = (4.0f + fin_cfg->traits.dorsal_height * powf(sinf((float)M_PI * u), fin_cfg->traits.dorsal_crest_power)) * fin_cfg->fin_scale;
            float sweep_back = fin_cfg->traits.dorsal_sweep * powf(u, 0.85f);

            /* Rosetail continuous petal ruffles */
            if (fin_cfg->traits.ruffle_amp > 0.1f) {
                h_fin += fin_cfg->traits.ruffle_amp * 0.7f * sinf(u * fin_cfg->traits.ruffle_freq);
            }

            /* Traveling hydrodynamic ripple wave (rich, fluid silk undulation) */
            float wave = 1.8f * sinf(fin_cfg->flow_phase - 2.5f * u);

            int y_tip = (int)roundf((float)y_base - h_fin + wave);
            int dy_span = y_base - y_tip;
            if (dy_span <= 0) continue;

            for (int y = y_tip; y < y_base; y++) {
                float v = (float)(y_base - y) / (float)dy_span;
                float px = (float)x + sweep_back * v;
                int ix = (int)roundf(px);
                int iy = y;

                if (ix < 0 || ix >= GRID_WIDTH || iy < 0 || iy >= GRID_HEIGHT) continue;

                uint8_t cur = get_pix(g, ix, iy, body_cfg);
                if (cur != PAL_BG && cur != PAL_SHADOW_DEEP && cur != PAL_SHADOW_MID) continue;

                /* Continuous crowntail spike webbing reduction */
                if (fin_cfg->traits.spike_protrusion > 0.05f) {
                    float web_cutoff = 1.0f - fin_cfg->traits.spike_protrusion;
                    if (v > web_cutoff && (ix % 3 != 0)) continue;
                }

                int is_ray = ((ix + iy / 2) % 3 == 0);
                uint8_t color = resolve_fin_color(v, is_ray, ix, iy, fin_cfg);

                /* Trailing edge translucency */
                if (v > 0.70f && ((ix ^ iy) & 1) && v > 0.90f) {
                    continue;
                }

                draw_pix(g, ix, iy, color, body_cfg);
            }
        }
    }

    /* 2. Anal Fin (Continuous silk drape) */
    if (fin_cfg->show_anal) {
        int a_xstart = x_start + (int)roundf(len * fin_cfg->traits.anal_base_start);
        int a_xend   = x_start + (int)roundf(len * fin_cfg->traits.anal_base_end);
        if (a_xend <= a_xstart) a_xend = a_xstart + 1;

        for (int x = a_xstart; x <= a_xend; x++) {
            int y_base = betta_get_body_bot_y(body_cfg, x);
            float u = (float)(x - a_xstart) / (float)(a_xend - a_xstart);

            float depth = (8.0f + fin_cfg->traits.anal_depth * sinf((float)M_PI * powf(u, 0.75f))) * fin_cfg->fin_scale;

            /* Continuous rosetail ruffle */
            if (fin_cfg->traits.ruffle_amp > 0.1f) {
                depth += fin_cfg->traits.ruffle_amp * 0.7f * sinf(u * fin_cfg->traits.ruffle_freq);
            }

            float sweep_back = fin_cfg->traits.anal_sweep * u;
            float wave = 1.8f * sinf(fin_cfg->flow_phase - 2.8f * u);

            int y_tip = (int)roundf((float)y_base + depth + wave);
            int dy_span = y_tip - y_base;
            if (dy_span <= 0) continue;

            for (int y = y_base + 1; y <= y_tip; y++) {
                float v = (float)(y - y_base) / (float)dy_span;
                float px = (float)x + sweep_back * v;
                int ix = (int)roundf(px);
                int iy = y;

                if (ix < 0 || ix >= GRID_WIDTH || iy < 0 || iy >= GRID_HEIGHT) continue;

                uint8_t cur = get_pix(g, ix, iy, body_cfg);
                if (cur != PAL_BG && cur != PAL_SHADOW_DEEP && cur != PAL_SHADOW_MID) continue;

                /* Continuous spike reduction */
                if (fin_cfg->traits.spike_protrusion > 0.05f) {
                    float web_cutoff = 1.0f - fin_cfg->traits.spike_protrusion;
                    if (v > web_cutoff && (ix % 3 != 0)) continue;
                }

                int is_ray = ((ix - iy / 2) % 3 == 0);
                uint8_t color = resolve_fin_color(v, is_ray, ix, iy, fin_cfg);

                draw_pix(g, ix, iy, color, body_cfg);
            }
        }
    }

    /* 3. Caudal Fin (Continuous Harmonic Spectrum) */
    if (fin_cfg->show_caudal) {
        int x_base = x_end - 1;
        int y_base = betta_get_spine_y(body_cfg, x_base);

        int num_rays = (fin_cfg->traits.caudal_rays >= 16) ? fin_cfg->traits.caudal_rays : 30;
        float spread = (fin_cfg->traits.caudal_spread > 0.5f) ? fin_cfg->traits.caudal_spread : 2.45f;
        float base_len = (fin_cfg->traits.caudal_length > 10.0f) ? fin_cfg->traits.caudal_length : 38.0f;

        for (int i = 0; i < num_rays; i++) {
            float t = (float)i / (float)(num_rays - 1); /* [0..1] top to bottom */
            float angle = -spread * 0.5f + spread * t;

            /* Continuous Lobe & Cleft Harmonic */
            float cleft_factor = 1.0f;
            if (fin_cfg->traits.cleft_depth > 0.02f) {
                float dt = fabsf(t - 0.5f);
                cleft_factor = 1.0f - fin_cfg->traits.cleft_depth * expf(-fin_cfg->traits.cleft_sharpness * dt * dt * 25.0f);
            }

            /* Continuous Rosetail Petal Ruffles */
            float ruffle_mod = 0.0f;
            if (fin_cfg->traits.ruffle_amp > 0.05f) {
                ruffle_mod = fin_cfg->traits.ruffle_amp * sinf(t * (float)M_PI * fin_cfg->traits.ruffle_freq);
            }

            /* Fan curve profile with Spadetail and Delta modulations */
            float fan_curve = 7.5f * sinf((float)M_PI * t);
            if (fin_cfg->traits.spade_sharpness > 0.2f) {
                float dt = fabsf(t - 0.5f);
                fan_curve = 16.0f * expf(-dt * fin_cfg->traits.spade_sharpness * 6.5f);
            } else if (fin_cfg->traits.delta_angle > 0.2f) {
                float dt = fabsf(t - 0.5f);
                fan_curve = 9.0f * (1.0f - dt * fin_cfg->traits.delta_angle * 1.8f);
            } else if (fin_cfg->traits.cleft_depth > 0.35f) {
                fan_curve = 6.5f * fabsf(sinf(2.0f * (float)M_PI * t));
            }

            float max_len = (base_len + fan_curve + ruffle_mod) * cleft_factor * fin_cfg->fin_scale;
            int num_steps = (int)roundf(max_len);

            for (int step = 3; step < num_steps; step++) {
                float s = (float)step / (float)num_steps;

                /* Continuous waterfall gravity drape */
                float gravity = fin_cfg->traits.gravity_droop * (7.5f * s * s);

                /* Dynamic kinematic trailing whip wave with propagation lag (rich silk flutter) */
                float lag_phase = fin_cfg->flow_phase - 2.0f * (float)M_PI * 0.85f - 1.8f * s;
                float flutter = 2.6f * s * sinf(lag_phase + angle * 0.7f);

                float px = (float)x_base + (float)step * cosf(angle) + 4.0f * s * s;
                float py = (float)y_base + (t - 0.5f) * 6.0f + (float)step * sinf(angle) + flutter + gravity;

                int ix = (int)roundf(px);
                int iy = (int)roundf(py);

                if (ix < 0 || ix >= GRID_WIDTH || iy < 0 || iy >= GRID_HEIGHT) continue;

                uint8_t cur = get_pix(g, ix, iy, body_cfg);
                if (cur != PAL_BG && cur != PAL_SHADOW_DEEP && cur != PAL_SHADOW_MID) continue;

                /* Continuous crowntail spike webbing reduction with double-ray fork */
                if (fin_cfg->traits.spike_protrusion > 0.05f) {
                    float web_cutoff = 1.0f - fin_cfg->traits.spike_protrusion;
                    if (s > web_cutoff) {
                        if (fin_cfg->traits.double_ray_fork) {
                            if ((i % 4 != 0) && (i % 4 != 1)) continue;
                            float fork_shift = (s - web_cutoff) * 2.2f;
                            py += (i % 4 == 0) ? -fork_shift : +fork_shift;
                            iy = (int)roundf(py);
                        } else {
                            if (i % 2 != 0) continue;
                        }
                    }
                }

                int is_rib = (i % 2 == 0);
                uint8_t color = resolve_fin_color(s, is_rib, ix, iy, fin_cfg);

                draw_pix(g, ix, iy, color, body_cfg);
            }
        }

        /* Peduncle seam blend */
        for (int dy = -4; dy <= 4; dy++) {
            draw_pix(g, x_base, y_base + dy, PAL_OUTLINE, body_cfg);
            draw_pix(g, x_base + 1, y_base + dy, PAL_BODY_DARK, body_cfg);
        }
    }

    /* 4. Foreground Pelvic Feeler(s) */
    if (fin_cfg->show_ventrals) {
        int x0 = body_cfg->center_x - 17;
        int y0 = betta_get_spine_y(body_cfg, x0) + 8;
        int num_pts = (int)roundf(fin_cfg->traits.ventral_length);
        if (num_pts < 16) num_pts = 16;
        float curl = fin_cfg->traits.ventral_curl;

        for (int i = 0; i < num_pts; i++) {
            float t = (float)i / (float)num_pts;
            float px, py;
            if (fin_cfg->traits.ventral_style == VENTRAL_CURLED) {
                px = (float)x0 + curl * 1.6f * sinf(t * 3.0f);
                py = (float)y0 + (float)num_pts * 0.80f * t;
            } else {
                px = (float)x0 + curl * t * t + 1.4f * sinf(fin_cfg->flow_phase + t * 4.5f);
                py = (float)y0 + (float)num_pts * 0.85f * t;
            }

            int ix = (int)roundf(px);
            int iy = (int)roundf(py);

            if (ix >= 0 && ix < GRID_WIDTH && iy >= 0 && iy < GRID_HEIGHT) {
                uint8_t color = (t > 0.90f) ? PAL_PEARL_WHITE :
                                (t > 0.65f) ? PAL_ACCENT_GLINT :
                                (t > 0.35f) ? PAL_ACCENT_LIGHT : PAL_ACCENT_MID;
                draw_pix(g, ix, iy, color, body_cfg);
                draw_pix(g, ix + 1, iy, PAL_ACCENT_DARK, body_cfg);
            }
        }

        /* Forked secondary feeler */
        if (fin_cfg->traits.ventral_fork_ratio > 0.1f) {
            int fork_pts = (int)roundf((float)num_pts * fin_cfg->traits.ventral_fork_ratio);
            for (int i = 0; i < fork_pts; i++) {
                float t = (float)i / (float)fork_pts;
                float px = (float)x0 + (curl * 1.8f + 5.0f) * t * t;
                float py = (float)y0 + (float)fork_pts * 0.85f * t;
                int ix = (int)roundf(px);
                int iy = (int)roundf(py);
                if (ix >= 0 && ix < GRID_WIDTH && iy >= 0 && iy < GRID_HEIGHT) {
                    uint8_t color = (t > 0.85f) ? PAL_PEARL_WHITE : PAL_ACCENT_LIGHT;
                    draw_pix(g, ix, iy, color, body_cfg);
                }
            }
        }
    }

    /* 5. Pectoral Fin with Active Flapping Kinematics */
    if (fin_cfg->show_pectoral) {
        int x0 = body_cfg->center_x - 13;
        int y0 = betta_get_spine_y(body_cfg, x0);

        /* Bettas flutter pectorals gently for stabilization */
        float flap = sinf(fin_cfg->flow_phase * 2.0f);
        int flap_dx = (int)roundf(flap * 1.3f);

        int max_w = (int)roundf(fin_cfg->traits.pectoral_width);
        int max_h = (int)roundf(fin_cfg->traits.pectoral_height);

        for (int dy = -max_h; dy <= max_h; dy++) {
            for (int dx = 0; dx <= max_w; dx++) {
                float dist = sqrtf((float)(dx * dx * 1.0f + dy * dy * 2.4f));
                if (dist > (float)max_w) continue;

                /* Continuous ruffle on rim */
                if (fin_cfg->traits.pectoral_ruffle > 0.1f) {
                    float ruff = fin_cfg->traits.pectoral_ruffle * sinf((float)dy * 1.5f);
                    if (dist > (float)max_w - 1.2f + ruff) continue;
                }

                int ix = x0 + dx + flap_dx;
                int iy = y0 + dy;

                if (ix < 0 || ix >= GRID_WIDTH || iy < 0 || iy >= GRID_HEIGHT) continue;
                uint8_t cur = get_pix(g, ix, iy, body_cfg);
                if (cur == PAL_BG || cur == PAL_OUTLINE) continue;

                uint8_t col;
                float norm_dist = dist / (float)max_w;
                if (fin_cfg->traits.pectoral_style == PECTORAL_DUMBO) {
                    if (norm_dist > 0.72f) {
                        col = ((ix + iy) % 2 == 0) ? PAL_PEARL_WHITE : PAL_IRIDESCENT_GLINT;
                    } else if (norm_dist > 0.44f) {
                        col = (dy % 2 == 0) ? PAL_HIGHLIGHT_BRIGHT : PAL_HIGHLIGHT_SOFT;
                    } else {
                        col = PAL_BODY_LIGHT;
                    }
                } else {
                    if (norm_dist > 0.78f) {
                        col = ((ix + iy) % 2 == 0) ? PAL_PEARL_WHITE : PAL_IRIDESCENT_GLINT;
                    } else if (norm_dist > 0.50f) {
                        col = (dy % 2 == 0) ? PAL_HIGHLIGHT_BRIGHT : PAL_HIGHLIGHT_SOFT;
                    } else {
                        col = ((ix ^ iy) & 1) ? PAL_HIGHLIGHT_SOFT : PAL_BODY_LIGHT;
                    }
                }

                draw_pix(g, ix, iy, col, body_cfg);
            }
        }
    }
}
