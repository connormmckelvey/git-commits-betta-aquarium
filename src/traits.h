#ifndef TRAITS_H
#define TRAITS_H

#include <stdint.h>

/* Phenotype classification categories (for human-readable display & taxonomy) */
typedef enum {
    FIN_STYLE_HALFMOON = 0,
    FIN_STYLE_OVER_HALFMOON,      /* Massive >180 degree expansive flare */
    FIN_STYLE_VEILTAIL,           /* Cascading waterfall gravity drape */
    FIN_STYLE_CROWNTAIL,          /* Needle-ray spikes with webbing reduction */
    FIN_STYLE_DOUBLETAIL,         /* Twin distinct caudal lobes + wide dorsal */
    FIN_STYLE_ROSETAIL,           /* Overlapping petal ruffles */
    FIN_STYLE_PLAKAT,             /* Short fighter fan, agile muscular blade */
    FIN_STYLE_SPADETAIL,          /* Sharp diamond/spearhead spade point */
    FIN_STYLE_DELTA,              /* Triangular expanding fan */
    FIN_STYLE_PROCEDURAL_HYBRID
} FinStyle;

typedef enum {
    BODY_ARCHETYPE_ROYAL = 0,    /* High crown crest, deep robust chest */
    BODY_ARCHETYPE_SLEEK,        /* Streamlined, agile, slender hydrodynamic torpedo */
    BODY_ARCHETYPE_DRAGON,       /* Muscular dragon body with heavy armored scales */
    BODY_ARCHETYPE_COMPACT,      /* Cute, rounded, baby-faced compact body */
    BODY_ARCHETYPE_GIANT,        /* Heavyweight brawny muscular giant */
    BODY_ARCHETYPE_FIGHTER       /* Athletic, taut, agile predator build */
} BodyArchetype;

typedef enum {
    FIN_PATTERN_SOLID = 0,       /* Smooth organic gradient base-to-tip */
    FIN_PATTERN_BUTTERFLY,       /* Two-tone: body base with pearl-white outer band */
    FIN_PATTERN_STRIATED,        /* High-contrast ray ribs and shadowed webbing */
    FIN_PATTERN_EDGED,           /* Luminous iridescent neon rim lining */
    FIN_PATTERN_MOTTLED          /* Organic marbled tone clusters across fins */
} FinPattern;

typedef enum {
    BODY_PATTERN_METALLIC = 0,   /* Uniform shimmering scale mesh */
    BODY_PATTERN_DRAGON_ARMOR,   /* Bold 3D diamond scales with deep pocket creases */
    BODY_PATTERN_MARBLE,         /* Organic swirling marbled tone patches */
    BODY_PATTERN_GRIZZLE,        /* Fine stippled salt-and-pepper micro-speckles */
    BODY_PATTERN_MASKED          /* Metallic dragon helmet mask covering head & gills */
} BodyPattern;

typedef enum {
    PECTORAL_STANDARD = 0,       /* Clean translucent mid-size paddle */
    PECTORAL_DUMBO               /* Large "Big-Ear" ruffled elephant-ear fan */
} PectoralStyle;

typedef enum {
    VENTRAL_STREAMER = 0,        /* Classic long elegant streamer */
    VENTRAL_FORKED,              /* Forked twin-streamer feelers */
    VENTRAL_CURLED               /* S-curve backward flowing curl */
} VentralStyle;

typedef enum {
    SNOUT_UPTURNED = 0,          /* Classic Betta surface-feeder scoop */
    SNOUT_ROMAN,                 /* Aristocratic arched curve */
    SNOUT_ARROW,                 /* Sharp hunting arrow profile */
    SNOUT_BULLDOG                /* Muscular pug chin with fighter underbite */
} SnoutStyle;

typedef enum {
    EYE_STYLE_CLASSIC = 0,       /* Standard expressive 6x6 glass eye */
    EYE_STYLE_SLEEK,             /* Fierce, slightly almond socket with intense iris */
    EYE_STYLE_WIDE               /* Large, curious round eye with extra catchlights */
} EyeStyle;

/* Procedural Genetic Traits derived deterministically from Seed Text.
 * Strictly decoupled: Controls shape, fins, morphology, and patterns, but NEVER colors! */
typedef struct {
    char          seed_name[64];       /* Name of the seed */
    uint64_t      seed_hash;           /* 64-bit seed hash */

    /* Qualitative Classification (for logging / display) */
    FinStyle      fin_style;           /* Dominant fin phenotype classification */
    BodyArchetype body_archetype;      /* Royal, Sleek, Dragon, Compact, Giant, Fighter */
    FinPattern    fin_pattern;         /* Solid, Butterfly, Striated, Edged, Mottled */
    BodyPattern   body_pattern;        /* Metallic, Dragon Armor, Marble, Grizzle, Masked */
    PectoralStyle pectoral_style;      /* Standard vs Dumbo / Big-Ear */
    VentralStyle  ventral_style;       /* Streamer, Forked, Curled */
    SnoutStyle    snout_style;         /* Upturned, Roman, Arrow, Bulldog */
    EyeStyle      eye_style;           /* Classic, Sleek, Wide */

    /* === CONTINUOUS PROCEDURAL FIN METRICS (Infinite Spectrum) === */
    /* Caudal (Tail) Continuous Spectrum */
    float         caudal_spread;       /* Fan angle spread in radians (1.40 to 3.40 rad) */
    float         caudal_length;       /* Base tail length in pixels (20.0 to 52.0 px) */
    int           caudal_rays;         /* Number of primary tail rays (16 to 42 rays) */
    float         lobe_count;          /* 1.0 (single fan), 2.0 (twin lobes), or partial cleft */
    float         cleft_depth;         /* Depth of cleft between lobes (0.0 to 0.70) */
    float         cleft_sharpness;     /* Steepness of center indentation (4.0 to 14.0) */
    float         ruffle_amp;          /* Petal ruffle amplitude (0.0 to 4.5 px) */
    float         ruffle_freq;         /* Petal ruffle wave frequency (4.0 to 18.0 cycles) */
    float         spike_protrusion;    /* Webbing reduction ratio / needle spikes (0.0 to 0.50) */
    int           double_ray_fork;     /* 1: crowntail rays fork into twin needles */
    float         gravity_droop;       /* Downward waterfall drape factor (0.0 to 1.25) */
    float         ray_boldness;        /* Thickness/contrast of rib rays (0.8 to 1.8) */
    float         spade_sharpness;     /* Spade-tail spearhead taper (0.0 to 2.2) */
    float         delta_angle;         /* Delta triangular fan angle (0.0 to 1.0) */

    /* Fin Patterns (Butterfly, Edge Rim, Marbled) */
    float         butterfly_band;      /* Butterfly white band start ratio (0.55 to 0.82) */
    float         edge_rim_glow;       /* Outer edge rim highlight thickness */

    /* Dorsal (Top Sail) Continuous Spectrum */
    float         dorsal_base_start;   /* Start ratio along body (0.22 to 0.44) */
    float         dorsal_base_end;     /* End ratio along body (0.84 to 0.96) */
    float         dorsal_height;       /* Crest height (10.0 to 32.0 px) */
    float         dorsal_sweep;        /* Backward curvature (4.0 to 24.0 px) */
    float         dorsal_crest_power;  /* Arching exponent: 0.5 (round sail) to 1.2 (banner) */

    /* Anal (Bottom Curtain) Continuous Spectrum */
    float         anal_base_start;     /* Start ratio along body (0.34 to 0.44) */
    float         anal_base_end;       /* End ratio along body (0.88 to 0.98) */
    float         anal_depth;          /* Curtain depth (10.0 to 40.0 px) */
    float         anal_sweep;          /* Backward drape sweep (5.0 to 20.0 px) */

    /* Pectoral & Pelvic Continuous Spectrum */
    float         pectoral_width;      /* Flank fan width (6.0 to 18.0 px) */
    float         pectoral_height;     /* Flank fan height (4.0 to 10.0 px) */
    float         pectoral_ruffle;     /* Big-Ear scalloped ruffle amplitude (0.0 to 2.5 px) */
    float         ventral_length;      /* Pelvic feeler length (18.0 to 46.0 px) */
    float         ventral_curl;        /* S-curl factor (-6.0 to 16.0 px) */
    float         ventral_fork_ratio;  /* 0.0 (single streamer) to 1.0 (twin diverging tines) */

    /* Body Silhouette Metrics */
    int           body_length;         /* Total body length in pixels (52 to 82 px) */
    float         crown_height;        /* Forehead hump height (10.0 to 17.5 px) */
    float         belly_depth;         /* Body depth multiplier (0.82 to 1.45) */
    float         peduncle_depth;      /* Tail base thickness (2.2 to 5.2 px) */
    float         snout_upturn;        /* Snout angle factor */
    float         jaw_underbite;       /* Muscular jaw protrusion */

    /* Scale & Texture Traits */
    int           scale_step;          /* Scale tile size (3 = fine, 4 = medium, 5 = broad dragon) */
    int           scale_density;       /* Scale relief intensity (subtle vs bold) */
    int           dorsal_shimmer_mode; /* 0: crystal glints, 1: continuous beam, 2: starry clusters */
    int           gill_curve_style;    /* Crescent shape variations */
    float         marble_seed_offset;  /* Organic marble pattern coordinate */
    float         mask_coverage;       /* 0.0 (no mask) to 1.0 (full metallic helmet mask) */
} BettaTraits;

/* Generate deterministic traits from any seed string (username, word, etc.) */
BettaTraits betta_traits_from_seed(const char* seed_text);

/* String descriptions for logging and UI */
const char* fin_style_name(FinStyle s);
const char* body_archetype_name(BodyArchetype b);
const char* fin_pattern_name(FinPattern p);
const char* body_pattern_name(BodyPattern b);
const char* pectoral_style_name(PectoralStyle p);
const char* ventral_style_name(VentralStyle v);
const char* snout_style_name(SnoutStyle s);
const char* eye_style_name(EyeStyle e);

#endif /* TRAITS_H */
