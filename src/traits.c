#include "traits.h"
#include <string.h>
#include <math.h>

/* 64-bit FNV-1a hash */
static uint64_t hash_str(const char* str) {
    uint64_t hash = 14695981039346656037ULL;
    if (!str || !*str) return hash;
    while (*str) {
        hash ^= (uint64_t)(unsigned char)(*str++);
        hash *= 1099511628211ULL;
    }
    return hash;
}

/* SplitMix64 deterministic PRNG */
typedef struct {
    uint64_t state;
} PRNG;

static uint64_t prng_next(PRNG* rng) {
    uint64_t z = (rng->state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static float prng_float(PRNG* rng, float min, float max) {
    uint64_t r = prng_next(rng);
    float norm = (float)(r & 0xFFFFFFFF) / (float)0xFFFFFFFF;
    return min + norm * (max - min);
}

static int prng_int(PRNG* rng, int min, int max) {
    if (min >= max) return min;
    uint64_t r = prng_next(rng);
    return min + (int)(r % (uint64_t)(max - min + 1));
}

BettaTraits betta_traits_from_seed(const char* seed_text) {
    BettaTraits t;
    memset(&t, 0, sizeof(t));

    const char* actual_seed = (seed_text && *seed_text) ? seed_text : "git-betta-default";
    strncpy(t.seed_name, actual_seed, sizeof(t.seed_name) - 1);
    t.seed_hash = hash_str(actual_seed);

    PRNG rng;
    rng.state = t.seed_hash;

    /* 1. Body Archetype & Proportions */
    int body_pick = prng_int(&rng, 0, 99);
    if (body_pick < 20) {
        t.body_archetype = BODY_ARCHETYPE_ROYAL;
        t.body_length    = prng_int(&rng, 62, 68);
        t.crown_height   = prng_float(&rng, 14.0f, 16.5f);
        t.belly_depth    = prng_float(&rng, 1.12f, 1.28f);
        t.peduncle_depth = prng_float(&rng, 3.4f, 4.4f);
    } else if (body_pick < 40) {
        t.body_archetype = BODY_ARCHETYPE_SLEEK;
        t.body_length    = prng_int(&rng, 66, 72);
        t.crown_height   = prng_float(&rng, 10.0f, 12.2f);
        t.belly_depth    = prng_float(&rng, 0.84f, 0.98f);
        t.peduncle_depth = prng_float(&rng, 2.2f, 3.0f);
    } else if (body_pick < 58) {
        t.body_archetype = BODY_ARCHETYPE_DRAGON;
        t.body_length    = prng_int(&rng, 70, 76);
        t.crown_height   = prng_float(&rng, 13.0f, 15.2f);
        t.belly_depth    = prng_float(&rng, 1.05f, 1.20f);
        t.peduncle_depth = prng_float(&rng, 3.5f, 4.6f);
    } else if (body_pick < 72) {
        t.body_archetype = BODY_ARCHETYPE_COMPACT;
        t.body_length    = prng_int(&rng, 52, 58);
        t.crown_height   = prng_float(&rng, 12.0f, 14.2f);
        t.belly_depth    = prng_float(&rng, 1.22f, 1.42f);
        t.peduncle_depth = prng_float(&rng, 2.8f, 3.6f);
    } else if (body_pick < 86) {
        t.body_archetype = BODY_ARCHETYPE_GIANT;
        t.body_length    = prng_int(&rng, 74, 82);
        t.crown_height   = prng_float(&rng, 15.0f, 17.5f);
        t.belly_depth    = prng_float(&rng, 1.25f, 1.45f);
        t.peduncle_depth = prng_float(&rng, 4.2f, 5.2f);
    } else {
        t.body_archetype = BODY_ARCHETYPE_FIGHTER;
        t.body_length    = prng_int(&rng, 58, 64);
        t.crown_height   = prng_float(&rng, 11.0f, 13.0f);
        t.belly_depth    = prng_float(&rng, 0.92f, 1.05f);
        t.peduncle_depth = prng_float(&rng, 3.0f, 3.8f);
    }

    /* 2. Snout Style & Profile */
    int snout_pick = prng_int(&rng, 0, 99);
    if (snout_pick < 35)      t.snout_style = SNOUT_UPTURNED;
    else if (snout_pick < 65) t.snout_style = SNOUT_ROMAN;
    else if (snout_pick < 85) t.snout_style = SNOUT_ARROW;
    else                      t.snout_style = SNOUT_BULLDOG;
    t.snout_upturn = prng_float(&rng, 0.85f, 1.35f);
    t.jaw_underbite = (t.snout_style == SNOUT_BULLDOG) ? prng_float(&rng, 1.0f, 1.6f) : 0.0f;

    /* 3. Eye Expression */
    if (t.body_archetype == BODY_ARCHETYPE_COMPACT) {
        t.eye_style = EYE_STYLE_WIDE;
    } else if (t.body_archetype == BODY_ARCHETYPE_FIGHTER) {
        t.eye_style = EYE_STYLE_SLEEK;
    } else {
        int eye_pick = prng_int(&rng, 0, 99);
        if (eye_pick < 40)      t.eye_style = EYE_STYLE_CLASSIC;
        else if (eye_pick < 72) t.eye_style = EYE_STYLE_SLEEK;
        else                    t.eye_style = EYE_STYLE_WIDE;
    }
    t.gill_curve_style = prng_int(&rng, 0, 2);

    /* 4. Scales Pattern, Shimmer & Texture */
    int bpat_roll = prng_int(&rng, 0, 99);
    if (t.body_archetype == BODY_ARCHETYPE_DRAGON) {
        t.body_pattern = (bpat_roll < 45) ? BODY_PATTERN_DRAGON_ARMOR : BODY_PATTERN_MASKED;
    } else if (bpat_roll < 25) {
        t.body_pattern = BODY_PATTERN_MARBLE;
    } else if (bpat_roll < 45) {
        t.body_pattern = BODY_PATTERN_DRAGON_ARMOR;
    } else if (bpat_roll < 65) {
        t.body_pattern = BODY_PATTERN_GRIZZLE;
    } else if (bpat_roll < 80) {
        t.body_pattern = BODY_PATTERN_MASKED;
    } else {
        t.body_pattern = BODY_PATTERN_METALLIC;
    }

    t.marble_seed_offset = prng_float(&rng, 0.0f, 64.0f);
    t.mask_coverage      = (t.body_pattern == BODY_PATTERN_MASKED) ? prng_float(&rng, 0.5f, 1.0f) : 0.0f;

    if (t.body_pattern == BODY_PATTERN_DRAGON_ARMOR || t.body_archetype == BODY_ARCHETYPE_DRAGON) {
        t.scale_step = 5;
    } else {
        int scale_pick = prng_int(&rng, 0, 2);
        t.scale_step = (scale_pick == 0) ? 3 : (scale_pick == 1 ? 4 : 5);
    }
    t.scale_density = prng_int(&rng, 1, 3);
    t.dorsal_shimmer_mode = prng_int(&rng, 0, 2);

    /* 5. Fin Color Patterning (Decoupled from Color Theme: controls texture/marking distribution) */
    int fpat_roll = prng_int(&rng, 0, 99);
    if (fpat_roll < 28) {
        t.fin_pattern    = FIN_PATTERN_BUTTERFLY; /* Two-tone pearl outer band */
        t.butterfly_band = prng_float(&rng, 0.62f, 0.78f);
    } else if (fpat_roll < 50) {
        t.fin_pattern    = FIN_PATTERN_EDGED;     /* Luminous rim glow */
        t.edge_rim_glow  = prng_float(&rng, 0.8f, 1.4f);
    } else if (fpat_roll < 72) {
        t.fin_pattern    = FIN_PATTERN_STRIATED;  /* High contrast ray ribs */
    } else if (fpat_roll < 88) {
        t.fin_pattern    = FIN_PATTERN_MOTTLED;   /* Organic marbled patches */
    } else {
        t.fin_pattern    = FIN_PATTERN_SOLID;     /* Pure velvety gradient */
    }

    /* 6. CONTINUOUS PROCEDURAL FIN MORPHOLOGY (Infinite Spectrum) */
    int is_plakat = (t.body_archetype == BODY_ARCHETYPE_FIGHTER) || (prng_int(&rng, 0, 99) < 18);

    /* Caudal Spread & Length */
    if (is_plakat) {
        t.caudal_spread = prng_float(&rng, 1.90f, 2.70f);
        t.caudal_length = prng_float(&rng, 22.0f, 28.0f); /* Short fighter blade */
        t.caudal_rays   = prng_int(&rng, 18, 28);
    } else {
        t.caudal_spread = prng_float(&rng, 1.70f, 3.35f); /* Up to Over-Halfmoon >190 deg */
        t.caudal_length = prng_float(&rng, 34.0f, 50.0f);
        t.caudal_rays   = prng_int(&rng, 24, 40);
    }

    /* Spade Tail Sharpness */
    int spade_roll = prng_int(&rng, 0, 99);
    if (spade_roll < 20 && !is_plakat) {
        t.spade_sharpness = prng_float(&rng, 1.0f, 2.2f);
    } else {
        t.spade_sharpness = 0.0f;
    }

    /* Delta Tail Angle */
    int delta_roll = prng_int(&rng, 0, 99);
    t.delta_angle = (delta_roll < 22) ? prng_float(&rng, 0.4f, 0.85f) : 0.0f;

    /* Lobe Count & Cleft Modulation (Twin lobes vs single fan) */
    int lobe_roll = prng_int(&rng, 0, 99);
    if (lobe_roll < 25 && t.spade_sharpness < 0.2f) {
        t.lobe_count      = 2.0f;
        t.cleft_depth     = prng_float(&rng, 0.42f, 0.68f);
        t.cleft_sharpness = prng_float(&rng, 6.0f, 13.0f);
    } else if (lobe_roll < 45 && t.spade_sharpness < 0.2f) {
        t.lobe_count      = 2.0f;
        t.cleft_depth     = prng_float(&rng, 0.12f, 0.32f);
        t.cleft_sharpness = prng_float(&rng, 4.0f, 8.0f);
    } else {
        t.lobe_count      = 1.0f;
        t.cleft_depth     = 0.0f;
        t.cleft_sharpness = 4.0f;
    }

    /* Petal Ruffle Modulation (Smooth to ruffled Rose-petal silk) */
    int ruffle_roll = prng_int(&rng, 0, 99);
    if (ruffle_roll < 28 && !is_plakat) {
        t.ruffle_amp  = prng_float(&rng, 2.2f, 4.2f);   /* Heavy rosetail silk folds */
        t.ruffle_freq = prng_float(&rng, 10.0f, 17.0f);
    } else if (ruffle_roll < 60) {
        t.ruffle_amp  = prng_float(&rng, 0.8f, 1.8f);   /* Delicate waves */
        t.ruffle_freq = prng_float(&rng, 6.0f, 12.0f);
    } else {
        t.ruffle_amp  = 0.0f;                           /* Clean smooth contour */
        t.ruffle_freq = 6.0f;
    }

    /* Webbing Reduction & Needle Ray Spikes (Smooth to Crowntail) */
    int spike_roll = prng_int(&rng, 0, 99);
    if (spike_roll < 26) {
        t.spike_protrusion = prng_float(&rng, 0.26f, 0.46f); /* Crowntail needle rays */
        t.double_ray_fork  = (prng_int(&rng, 0, 1) == 1);     /* Forked double rays */
    } else if (spike_roll < 52) {
        t.spike_protrusion = prng_float(&rng, 0.08f, 0.20f); /* Serrated / combtail tips */
        t.double_ray_fork  = 0;
    } else {
        t.spike_protrusion = 0.0f;                           /* Full continuous silk webbing */
        t.double_ray_fork  = 0;
    }

    /* Gravity Drape / Waterfall Droop (Buoyant fan vs Veiltail) */
    int droop_roll = prng_int(&rng, 0, 99);
    if (droop_roll < 26 && !is_plakat && t.spade_sharpness < 0.2f) {
        t.gravity_droop = prng_float(&rng, 0.72f, 1.20f); /* Deep veiltail waterfall cascade */
        t.caudal_length = prng_float(&rng, 44.0f, 54.0f); /* Extra long trailing plume */
    } else if (droop_roll < 58) {
        t.gravity_droop = prng_float(&rng, 0.20f, 0.58f); /* Gentle fluid drift */
    } else {
        t.gravity_droop = 0.0f;                           /* Flared buoyant crest */
    }

    t.ray_boldness = prng_float(&rng, 0.9f, 1.7f);

    /* 7. Dorsal Sail Continuous Parameters */
    if (t.cleft_depth > 0.35f) {
        /* Doubletail genetics widen the dorsal base forward across the back */
        t.dorsal_base_start = prng_float(&rng, 0.22f, 0.28f);
    } else {
        t.dorsal_base_start = prng_float(&rng, 0.35f, 0.43f);
    }
    t.dorsal_base_end = prng_float(&rng, 0.88f, 0.95f);

    if (is_plakat) {
        t.dorsal_height = prng_float(&rng, 10.0f, 16.0f); /* Compact fighter crest */
        t.dorsal_sweep  = prng_float(&rng, 5.0f, 12.0f);
    } else {
        t.dorsal_height = prng_float(&rng, 16.0f, 30.0f);
        t.dorsal_sweep  = prng_float(&rng, 8.0f, 24.0f);
    }
    t.dorsal_crest_power = prng_float(&rng, 0.65f, 1.05f);

    /* 8. Anal Veil Continuous Parameters */
    t.anal_base_start = prng_float(&rng, 0.36f, 0.44f);
    t.anal_base_end   = prng_float(&rng, 0.88f, 0.96f);
    if (is_plakat) {
        t.anal_depth = prng_float(&rng, 12.0f, 18.0f); /* Sharp fighter saber */
        t.anal_sweep = prng_float(&rng, 8.0f, 14.0f);
    } else {
        t.anal_depth = prng_float(&rng, 18.0f, 34.0f);
        if (t.gravity_droop > 0.60f) {
            t.anal_depth *= 1.25f; /* Deep cascading curtain */
        }
        t.anal_sweep = prng_float(&rng, 7.0f, 18.0f);
    }

    /* 9. Pectoral & Pelvic Variety */
    int pec_roll = prng_int(&rng, 0, 99);
    if (pec_roll < 28) {
        t.pectoral_style  = PECTORAL_DUMBO;
        t.pectoral_width  = prng_float(&rng, 13.0f, 17.5f); /* Giant elephant ear */
        t.pectoral_height = prng_float(&rng, 7.0f, 9.5f);
        t.pectoral_ruffle = prng_float(&rng, 1.2f, 2.4f);
    } else {
        t.pectoral_style  = PECTORAL_STANDARD;
        t.pectoral_width  = prng_float(&rng, 7.0f, 9.5f);
        t.pectoral_height = prng_float(&rng, 4.0f, 5.5f);
        t.pectoral_ruffle = 0.0f;
    }

    int ventral_roll = prng_int(&rng, 0, 99);
    if (ventral_roll < 45) {
        t.ventral_style      = VENTRAL_STREAMER;
        t.ventral_fork_ratio = 0.0f;
    } else if (ventral_roll < 75) {
        t.ventral_style      = VENTRAL_FORKED;
        t.ventral_fork_ratio = prng_float(&rng, 0.65f, 1.0f);
    } else {
        t.ventral_style      = VENTRAL_CURLED;
        t.ventral_fork_ratio = 0.0f;
    }
    t.ventral_length = prng_float(&rng, 22.0f, 42.0f);
    t.ventral_curl   = prng_float(&rng, 5.0f, 14.0f);

    /* 10. Qualitative Classification (Convenience for human logs / UI) */
    if (t.spade_sharpness > 0.6f) {
        t.fin_style = FIN_STYLE_SPADETAIL;
    } else if (t.cleft_depth > 0.38f) {
        t.fin_style = FIN_STYLE_DOUBLETAIL;
    } else if (t.spike_protrusion > 0.24f) {
        t.fin_style = FIN_STYLE_CROWNTAIL;
    } else if (t.ruffle_amp > 2.0f) {
        t.fin_style = FIN_STYLE_ROSETAIL;
    } else if (t.gravity_droop > 0.68f) {
        t.fin_style = FIN_STYLE_VEILTAIL;
    } else if (is_plakat || t.caudal_length < 29.0f) {
        t.fin_style = FIN_STYLE_PLAKAT;
    } else if (t.caudal_spread > 3.05f) {
        t.fin_style = FIN_STYLE_OVER_HALFMOON;
    } else if (t.delta_angle > 0.4f) {
        t.fin_style = FIN_STYLE_DELTA;
    } else if (t.caudal_spread > 2.30f && t.caudal_length > 33.0f) {
        t.fin_style = FIN_STYLE_HALFMOON;
    } else {
        t.fin_style = FIN_STYLE_PROCEDURAL_HYBRID;
    }

    return t;
}

const char* fin_style_name(FinStyle s) {
    switch (s) {
        case FIN_STYLE_HALFMOON:          return "Halfmoon Fan";
        case FIN_STYLE_OVER_HALFMOON:     return "Over-Halfmoon Flare";
        case FIN_STYLE_VEILTAIL:          return "Veiltail Cascade";
        case FIN_STYLE_CROWNTAIL:         return "Crowntail Spikes";
        case FIN_STYLE_DOUBLETAIL:        return "Doubletail Twin-Lobe";
        case FIN_STYLE_ROSETAIL:          return "Rosetail Petal-Fringe";
        case FIN_STYLE_PLAKAT:            return "Plakat Fighter Blade";
        case FIN_STYLE_SPADETAIL:         return "Spadetail Diamond Point";
        case FIN_STYLE_DELTA:             return "Super Delta Fan";
        case FIN_STYLE_PROCEDURAL_HYBRID: return "Continuous Procedural Hybrid";
        default:                          return "Organic Procedural";
    }
}

const char* body_archetype_name(BodyArchetype b) {
    switch (b) {
        case BODY_ARCHETYPE_ROYAL:   return "Royal Crest";
        case BODY_ARCHETYPE_SLEEK:   return "Sleek Torpedo";
        case BODY_ARCHETYPE_DRAGON:  return "Armored Dragon";
        case BODY_ARCHETYPE_COMPACT: return "Compact Chubby";
        case BODY_ARCHETYPE_GIANT:   return "Muscular Giant";
        case BODY_ARCHETYPE_FIGHTER: return "Agile Predator";
        default:                     return "Classic";
    }
}

const char* fin_pattern_name(FinPattern p) {
    switch (p) {
        case FIN_PATTERN_BUTTERFLY: return "Butterfly Two-Tone";
        case FIN_PATTERN_EDGED:     return "Luminous Rim Edge";
        case FIN_PATTERN_STRIATED:  return "Striated Ray Ribs";
        case FIN_PATTERN_MOTTLED:   return "Marbled Mottling";
        default:                    return "Solid Silk Gradient";
    }
}

const char* body_pattern_name(BodyPattern b) {
    switch (b) {
        case BODY_PATTERN_DRAGON_ARMOR: return "3D Dragon Armor";
        case BODY_PATTERN_MARBLE:       return "Koi Swirling Marble";
        case BODY_PATTERN_GRIZZLE:      return "Grizzle Salt-and-Pepper";
        case BODY_PATTERN_MASKED:       return "Metallic Full Helmet Mask";
        default:                        return "Metallic Glint Scale";
    }
}

const char* pectoral_style_name(PectoralStyle p) {
    switch (p) {
        case PECTORAL_DUMBO: return "Dumbo Big-Ear";
        default:             return "Standard Paddle";
    }
}

const char* ventral_style_name(VentralStyle v) {
    switch (v) {
        case VENTRAL_FORKED: return "Forked Twin Streamer";
        case VENTRAL_CURLED: return "Curled S-Ribbon";
        default:             return "Classic Streamer";
    }
}

const char* snout_style_name(SnoutStyle s) {
    switch (s) {
        case SNOUT_ROMAN:   return "Roman Curve";
        case SNOUT_ARROW:   return "Arrow Hunting";
        case SNOUT_BULLDOG: return "Bulldog Fighter Underbite";
        default:            return "Upturned Lip";
    }
}

const char* eye_style_name(EyeStyle e) {
    switch (e) {
        case EYE_STYLE_SLEEK: return "Sleek Fierce";
        case EYE_STYLE_WIDE:  return "Wide Curious";
        default:              return "Classic Glass";
    }
}
