#ifndef PALETTE_H
#define PALETTE_H

#include "pixel_grid.h"

/* 16-Color Palette Indices for High-Fidelity Pixel Shading */
#define PAL_BG                0   /* Dark deep water background */
#define PAL_OUTLINE           1   /* Darkest silhouette contour */
#define PAL_SHADOW_DEEP       2   /* Deep ambient occlusion / belly shadow */
#define PAL_SHADOW_MID        3   /* Secondary ventral shadow */
#define PAL_BODY_DARK         4   /* Lower flank body tone */
#define PAL_BODY_MID          5   /* Core body midtone */
#define PAL_BODY_LIGHT        6   /* Upper flank bright tone */
#define PAL_HIGHLIGHT_SOFT    7   /* Dorsal slope highlight */
#define PAL_HIGHLIGHT_BRIGHT  8   /* Dorsal crest specular highlight */
#define PAL_IRIDESCENT_GLINT  9   /* Vibrant theme accent / scale glint */
#define PAL_PEARL_WHITE      10   /* Specular pearl shine */
#define PAL_EYE_SOCKET       11   /* Dark eye contour */
#define PAL_EYE_IRIS         12   /* Colored iris ring */
#define PAL_EYE_PUPIL        13   /* Solid dark pupil */
#define PAL_EYE_GLINT        14   /* White catchlight */
#define PAL_GILL_CREASE      15   /* Operculum shadow crease */

/* Multi-Color Extensions: Secondary & Tertiary Language Color Ramps */
#define PAL_FIN_DARK         16   /* Deep shadow of fin webbing */
#define PAL_FIN_MID          17   /* Core midtone of fin webbing */
#define PAL_FIN_LIGHT        18   /* Bright tone of fin rays */
#define PAL_FIN_HIGHLIGHT    19   /* Specular highlight of fin crest */
#define PAL_FIN_TIP          20   /* Outer edge accent */
#define PAL_ACCENT_DARK      21   /* Accent shadow / feeler base */
#define PAL_ACCENT_MID       22   /* Accent midtone */
#define PAL_ACCENT_LIGHT     23   /* Accent bright feeler tip */
#define PAL_ACCENT_GLINT     24   /* Luminous accent glint / butterfly band */

typedef enum {
    THEME_SAPPHIRE = 0, /* Electric Sapphire (Deep Blue / Cyan) */
    THEME_EMERALD,      /* Emerald Serpent (Jade / Mint)         */
    THEME_FLAME,        /* Flame Copper (Crimson / Fiery Amber)  */
    THEME_CYAN          /* Glacial Cyan (Teal / Frost White)     */
} PaletteTheme;

/* Load a specific color theme into the grid's palette */
void palette_load_theme(PixelGrid* g, PaletteTheme theme);

/* Construct a multi-colored palette from 3 colors (Body, Fins, Accents) */
void palette_from_colors(PixelGrid* g, Color body_col, Color fin_col, Color accent_col);

/* Lookup canonical GitHub language color by name (e.g. "C", "Python", "Rust", "Go") or #RRGGBB hex */
Color palette_find_language_color(const char* lang_name);

#endif /* PALETTE_H */
