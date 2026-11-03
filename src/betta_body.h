#ifndef BETTA_BODY_H
#define BETTA_BODY_H

#include "pixel_grid.h"
#include "traits.h"

typedef struct {
    int         center_x;       /* Center X of fish on grid */
    int         center_y;       /* Center Y of fish on grid */
    int         length;         /* Total body length in pixels */
    float       plumpness;      /* Belly fullness: 0.8 (sleek), 1.0 (normal), 1.25 (plump) */
    int         mouth_open;     /* 0: closed upturned lips, 1: feeding */
    int         eye_blink;      /* 0: open with catchlight, 1: sleeping slit */
    int         gill_flare;     /* 0: normal, 1: flaring display */
    float       swim_phase;     /* Kinematic swim undulation phase (0.0 to 2*PI) */
    float       swim_amp;       /* Spine undulation amplitude in pixels (default: 2.8f) */
    int         facing_left;    /* 1: facing left (default), 0: facing right */
    BettaTraits traits;         /* Procedural genetic traits from seed text */
} BettaBodyConfig;

/* Return default configuration for adult fish */
BettaBodyConfig betta_body_default_config(void);

/* Helpers to compute exact spine and top/bottom profile heights at column x */
int betta_get_spine_y(const BettaBodyConfig* bcfg, int x);
int betta_get_body_top_y(const BettaBodyConfig* bcfg, int x);
int betta_get_body_bot_y(const BettaBodyConfig* bcfg, int x);

/* Render the procedural Betta fish body into the pixel grid (with kinematics) */
void render_betta_body(PixelGrid* g, const BettaBodyConfig* cfg);

#endif /* BETTA_BODY_H */
