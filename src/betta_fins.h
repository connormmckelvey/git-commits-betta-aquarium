#ifndef BETTA_FINS_H
#define BETTA_FINS_H

#include "pixel_grid.h"
#include "betta_body.h"
#include "traits.h"

typedef struct {
    float       flow_phase;     /* Flow phase offset (0.0f for static, animated in phase 2) */
    float       pectoral_phase; /* Independent rapid pectoral fin flutter phase (default: 0 = 2.5x flow) */
    float       fin_scale;      /* Fin size scale factor (default: 1.0f) */
    int         show_caudal;    /* 1 to render caudal (tail) halfmoon fan */
    int         show_dorsal;    /* 1 to render dorsal top sail */
    int         show_anal;      /* 1 to render anal bottom veil */
    int         show_ventrals;  /* 1 to render pelvic thread feelers */
    int         show_pectoral;  /* 1 to render pectoral steering fan */
    BettaTraits traits;         /* Procedural genetic traits from seed text */
} BettaFinConfig;

/* Return default configuration for adult Halfmoon fins */
BettaFinConfig betta_fin_default_config(void);

/* Render background fins (far ventral feeler, background caudal folds) BEFORE body */
void render_betta_fins_bg(PixelGrid* g, const BettaBodyConfig* body_cfg, const BettaFinConfig* fin_cfg);

/* Render foreground fins (dorsal sail, anal veil, caudal fan, foreground feeler, pectoral) AFTER body */
void render_betta_fins_fg(PixelGrid* g, const BettaBodyConfig* body_cfg, const BettaFinConfig* fin_cfg);

#endif /* BETTA_FINS_H */
