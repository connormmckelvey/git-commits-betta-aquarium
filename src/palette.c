#include "palette.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static Color hex_color(uint8_t r, uint8_t g, uint8_t b) {
    Color c = { r, g, b, 255 };
    return c;
}

static Color shade_color(Color c, float factor, Color tint, float tint_mix) {
    float r = (float)c.r * factor * (1.0f - tint_mix) + (float)tint.r * tint_mix;
    float g = (float)c.g * factor * (1.0f - tint_mix) + (float)tint.g * tint_mix;
    float b = (float)c.b * factor * (1.0f - tint_mix) + (float)tint.b * tint_mix;
    if (r < 0.0f) r = 0.0f; else if (r > 255.0f) r = 255.0f;
    if (g < 0.0f) g = 0.0f; else if (g > 255.0f) g = 255.0f;
    if (b < 0.0f) b = 0.0f; else if (b > 255.0f) b = 255.0f;
    return (Color){ (uint8_t)r, (uint8_t)g, (uint8_t)b, 255 };
}

void palette_load_theme(PixelGrid* g, PaletteTheme theme) {
    if (!g) return;

    /* Universal eye & specular values */
    g->palette[PAL_PEARL_WHITE] = hex_color(248, 250, 252);
    g->palette[PAL_EYE_SOCKET]  = hex_color(2, 6, 23);
    g->palette[PAL_EYE_PUPIL]   = hex_color(0, 0, 0);
    g->palette[PAL_EYE_GLINT]   = hex_color(255, 255, 255);

    switch (theme) {
        case THEME_SAPPHIRE:
            g->palette[PAL_BG]                = hex_color(6, 11, 24);
            g->palette[PAL_OUTLINE]           = hex_color(3, 7, 18);
            g->palette[PAL_SHADOW_DEEP]       = hex_color(8, 21, 46);
            g->palette[PAL_SHADOW_MID]        = hex_color(13, 33, 70);
            g->palette[PAL_BODY_DARK]         = hex_color(18, 52, 105);
            g->palette[PAL_BODY_MID]          = hex_color(24, 76, 148);
            g->palette[PAL_BODY_LIGHT]        = hex_color(33, 105, 196);
            g->palette[PAL_HIGHLIGHT_SOFT]    = hex_color(59, 130, 246);
            g->palette[PAL_HIGHLIGHT_BRIGHT]  = hex_color(96, 165, 250);
            g->palette[PAL_IRIDESCENT_GLINT]  = hex_color(56, 189, 248);
            g->palette[PAL_EYE_IRIS]          = hex_color(29, 78, 216);
            g->palette[PAL_GILL_CREASE]       = hex_color(4, 12, 28);
            /* Fin ramp */
            g->palette[PAL_FIN_DARK]          = hex_color(14, 42, 88);
            g->palette[PAL_FIN_MID]           = hex_color(24, 76, 148);
            g->palette[PAL_FIN_LIGHT]         = hex_color(37, 115, 210);
            g->palette[PAL_FIN_HIGHLIGHT]     = hex_color(96, 165, 250);
            g->palette[PAL_FIN_TIP]           = hex_color(56, 189, 248);
            /* Accent ramp */
            g->palette[PAL_ACCENT_DARK]       = hex_color(12, 50, 95);
            g->palette[PAL_ACCENT_MID]        = hex_color(56, 189, 248);
            g->palette[PAL_ACCENT_LIGHT]      = hex_color(186, 242, 254);
            g->palette[PAL_ACCENT_GLINT]      = hex_color(224, 247, 255);
            break;

        case THEME_EMERALD:
            g->palette[PAL_BG]                = hex_color(4, 16, 12);
            g->palette[PAL_OUTLINE]           = hex_color(2, 10, 7);
            g->palette[PAL_SHADOW_DEEP]       = hex_color(6, 32, 22);
            g->palette[PAL_SHADOW_MID]        = hex_color(10, 52, 35);
            g->palette[PAL_BODY_DARK]         = hex_color(16, 80, 52);
            g->palette[PAL_BODY_MID]          = hex_color(22, 115, 74);
            g->palette[PAL_BODY_LIGHT]        = hex_color(30, 155, 98);
            g->palette[PAL_HIGHLIGHT_SOFT]    = hex_color(52, 211, 153);
            g->palette[PAL_HIGHLIGHT_BRIGHT]  = hex_color(110, 231, 183);
            g->palette[PAL_IRIDESCENT_GLINT]  = hex_color(167, 243, 208);
            g->palette[PAL_EYE_IRIS]          = hex_color(16, 185, 129);
            g->palette[PAL_GILL_CREASE]       = hex_color(4, 18, 12);
            /* Fin ramp */
            g->palette[PAL_FIN_DARK]          = hex_color(12, 60, 40);
            g->palette[PAL_FIN_MID]           = hex_color(22, 115, 74);
            g->palette[PAL_FIN_LIGHT]         = hex_color(34, 175, 112);
            g->palette[PAL_FIN_HIGHLIGHT]     = hex_color(110, 231, 183);
            g->palette[PAL_FIN_TIP]           = hex_color(167, 243, 208);
            /* Accent ramp */
            g->palette[PAL_ACCENT_DARK]       = hex_color(10, 70, 48);
            g->palette[PAL_ACCENT_MID]        = hex_color(52, 211, 153);
            g->palette[PAL_ACCENT_LIGHT]      = hex_color(167, 243, 208);
            g->palette[PAL_ACCENT_GLINT]      = hex_color(209, 250, 229);
            break;

        case THEME_FLAME:
            g->palette[PAL_BG]                = hex_color(18, 8, 10);
            g->palette[PAL_OUTLINE]           = hex_color(12, 4, 6);
            g->palette[PAL_SHADOW_DEEP]       = hex_color(50, 14, 16);
            g->palette[PAL_SHADOW_MID]        = hex_color(85, 20, 22);
            g->palette[PAL_BODY_DARK]         = hex_color(130, 28, 28);
            g->palette[PAL_BODY_MID]          = hex_color(180, 42, 32);
            g->palette[PAL_BODY_LIGHT]        = hex_color(225, 68, 38);
            g->palette[PAL_HIGHLIGHT_SOFT]    = hex_color(249, 115, 22);
            g->palette[PAL_HIGHLIGHT_BRIGHT]  = hex_color(251, 146, 60);
            g->palette[PAL_IRIDESCENT_GLINT]  = hex_color(253, 186, 116);
            g->palette[PAL_EYE_IRIS]          = hex_color(220, 38, 38);
            g->palette[PAL_GILL_CREASE]       = hex_color(28, 6, 8);
            /* Fin ramp */
            g->palette[PAL_FIN_DARK]          = hex_color(110, 24, 24);
            g->palette[PAL_FIN_MID]           = hex_color(180, 42, 32);
            g->palette[PAL_FIN_LIGHT]         = hex_color(235, 75, 42);
            g->palette[PAL_FIN_HIGHLIGHT]     = hex_color(251, 146, 60);
            g->palette[PAL_FIN_TIP]           = hex_color(253, 186, 116);
            /* Accent ramp */
            g->palette[PAL_ACCENT_DARK]       = hex_color(140, 35, 20);
            g->palette[PAL_ACCENT_MID]        = hex_color(249, 115, 22);
            g->palette[PAL_ACCENT_LIGHT]      = hex_color(253, 186, 116);
            g->palette[PAL_ACCENT_GLINT]      = hex_color(254, 243, 199);
            break;

        case THEME_CYAN:
        default:
            g->palette[PAL_BG]                = hex_color(5, 16, 22);
            g->palette[PAL_OUTLINE]           = hex_color(2, 9, 14);
            g->palette[PAL_SHADOW_DEEP]       = hex_color(8, 34, 46);
            g->palette[PAL_SHADOW_MID]        = hex_color(13, 54, 72);
            g->palette[PAL_BODY_DARK]         = hex_color(18, 80, 106);
            g->palette[PAL_BODY_MID]          = hex_color(22, 112, 146);
            g->palette[PAL_BODY_LIGHT]        = hex_color(30, 150, 192);
            g->palette[PAL_HIGHLIGHT_SOFT]    = hex_color(45, 212, 235);
            g->palette[PAL_HIGHLIGHT_BRIGHT]  = hex_color(103, 232, 249);
            g->palette[PAL_IRIDESCENT_GLINT]  = hex_color(186, 242, 254);
            g->palette[PAL_EYE_IRIS]          = hex_color(14, 165, 233);
            g->palette[PAL_GILL_CREASE]       = hex_color(4, 18, 26);
            /* Fin ramp */
            g->palette[PAL_FIN_DARK]          = hex_color(16, 68, 92);
            g->palette[PAL_FIN_MID]           = hex_color(22, 112, 146);
            g->palette[PAL_FIN_LIGHT]         = hex_color(32, 165, 210);
            g->palette[PAL_FIN_HIGHLIGHT]     = hex_color(103, 232, 249);
            g->palette[PAL_FIN_TIP]           = hex_color(186, 242, 254);
            /* Accent ramp */
            g->palette[PAL_ACCENT_DARK]       = hex_color(14, 72, 98);
            g->palette[PAL_ACCENT_MID]        = hex_color(45, 212, 235);
            g->palette[PAL_ACCENT_LIGHT]      = hex_color(186, 242, 254);
            g->palette[PAL_ACCENT_GLINT]      = hex_color(224, 247, 255);
            break;
    }
}

void palette_from_colors(PixelGrid* g, Color body_col, Color fin_col, Color accent_col) {
    if (!g) return;

    /* Base eye & specular values */
    g->palette[PAL_PEARL_WHITE] = hex_color(248, 250, 252);
    g->palette[PAL_EYE_SOCKET]  = hex_color(2, 6, 20);
    g->palette[PAL_EYE_PUPIL]   = hex_color(0, 0, 0);
    g->palette[PAL_EYE_GLINT]   = hex_color(255, 255, 255);

    /* Background and Outline tinted with deep ambient ocean tone */
    g->palette[PAL_BG]          = hex_color(8, 12, 22);
    g->palette[PAL_OUTLINE]     = shade_color(body_col, 0.12f, hex_color(2, 4, 10), 0.5f);

    /* Primary Language: Volumetric Body Ramp */
    g->palette[PAL_SHADOW_DEEP]      = shade_color(body_col, 0.28f, hex_color(4, 8, 18), 0.35f);
    g->palette[PAL_SHADOW_MID]       = shade_color(body_col, 0.46f, hex_color(8, 16, 32), 0.25f);
    g->palette[PAL_BODY_DARK]        = shade_color(body_col, 0.70f, hex_color(0, 0, 0), 0.0f);
    g->palette[PAL_BODY_MID]         = body_col;
    g->palette[PAL_BODY_LIGHT]       = shade_color(body_col, 1.25f, hex_color(255, 255, 255), 0.20f);
    g->palette[PAL_HIGHLIGHT_SOFT]   = shade_color(body_col, 1.45f, hex_color(255, 255, 255), 0.38f);
    g->palette[PAL_HIGHLIGHT_BRIGHT] = shade_color(body_col, 1.68f, hex_color(255, 255, 255), 0.55f);
    g->palette[PAL_GILL_CREASE]      = shade_color(body_col, 0.18f, hex_color(0, 0, 0), 0.0f);

    /* Secondary Language: Dedicated Flowing Fin Veil Ramp */
    g->palette[PAL_FIN_DARK]         = shade_color(fin_col, 0.48f, hex_color(6, 10, 20), 0.3f);
    g->palette[PAL_FIN_MID]          = fin_col;
    g->palette[PAL_FIN_LIGHT]        = shade_color(fin_col, 1.28f, hex_color(255, 255, 255), 0.20f);
    g->palette[PAL_FIN_HIGHLIGHT]    = shade_color(fin_col, 1.55f, hex_color(255, 255, 255), 0.42f);
    g->palette[PAL_FIN_TIP]          = shade_color(accent_col, 1.30f, hex_color(255, 255, 255), 0.25f);

    /* Tertiary / Accent Language: Streamers, Glints, Eye Iris & Butterfly Bands */
    g->palette[PAL_IRIDESCENT_GLINT] = accent_col;
    g->palette[PAL_EYE_IRIS]         = accent_col;
    g->palette[PAL_ACCENT_DARK]      = shade_color(accent_col, 0.45f, hex_color(6, 10, 20), 0.3f);
    g->palette[PAL_ACCENT_MID]       = accent_col;
    g->palette[PAL_ACCENT_LIGHT]     = shade_color(accent_col, 1.35f, hex_color(255, 255, 255), 0.25f);
    g->palette[PAL_ACCENT_GLINT]     = shade_color(accent_col, 1.65f, hex_color(255, 255, 255), 0.50f);
}

typedef struct {
    const char* name;
    Color       color;
} LangColorEntry;

static const LangColorEntry GITHUB_LANG_TABLE[] = {
    { "c",            { 85,  85,  85, 255 } }, /* #555555 Slate Grey */
    { "c++",          { 243, 75, 125, 255 } }, /* #F34B7D Magenta Pink */
    { "cpp",          { 243, 75, 125, 255 } },
    { "c#",           { 23, 134,   0, 255 } }, /* #178600 Green */
    { "csharp",       { 23, 134,   0, 255 } },
    { "python",       { 53, 114, 165, 255 } }, /* #3572A5 Python Blue */
    { "py",           { 53, 114, 165, 255 } },
    { "javascript",   { 241, 224,  90, 255 } }, /* #F1E05A Canary Gold */
    { "js",           { 241, 224,  90, 255 } },
    { "typescript",   { 49, 120, 198, 255 } }, /* #3178C6 Sapphire Blue */
    { "ts",           { 49, 120, 198, 255 } },
    { "rust",         { 222, 165, 132, 255 } }, /* #DEA584 Terracotta Rust */
    { "go",           {   0, 173, 216, 255 } }, /* #00ADD8 Cyan */
    { "golang",       {   0, 173, 216, 255 } },
    { "html",         { 227,  76,  38, 255 } }, /* #E34C26 Flame Orange */
    { "css",          {  86,  61, 124, 255 } }, /* #563D7C Royal Violet */
    { "ruby",         { 112,  21,  22, 255 } }, /* #701516 Ruby Crimson */
    { "java",         { 176, 114,  25, 255 } }, /* #B07219 Amber Bronze */
    { "php",          {  79,  93, 149, 255 } }, /* #4F5D95 Indigo */
    { "swift",        { 240,  81,  56, 255 } }, /* #F05138 Sunset Orange */
    { "kotlin",       { 169, 123, 255, 255 } }, /* #A97BFF Electric Violet */
    { "kt",           { 169, 123, 255, 255 } },
    { "shell",        { 137, 224,  81, 255 } }, /* #89E051 Lime Green */
    { "bash",         { 137, 224,  81, 255 } },
    { "sh",           { 137, 224,  81, 255 } },
    { "lua",          {   0,   0, 128, 255 } }, /* #000080 Deep Navy */
    { "dart",         {   0, 180, 171, 255 } }, /* #00B4AB Teal */
    { "zig",          { 236, 145,  92, 255 } }, /* #EC915C Peach */
    { "markdown",     {   8,  63, 161, 255 } }, /* #083FA1 Deep Ocean */
    { "md",           {   8,  63, 161, 255 } },
    { "r",            {  25, 140, 231, 255 } }, /* #198CE7 Sky Blue */
    { "scala",        { 194,  45,  38, 255 } }, /* #C22D26 Crimson */
    { "elixir",       { 110,  74, 126, 255 } }, /* #6E4A7E Purple */
    { "haskell",      {  94,  80, 134, 255 } }, /* #5E5086 Violet */
    { "clojure",      { 219,  88,  85, 255 } }, /* #DB5855 Coral */
    { "ocaml",        { 238, 106,  26, 255 } }, /* #EE6A1A Bright Orange */
    { "solidity",     { 170, 170, 170, 255 } }, /* #AAAAAA Silver */
    { NULL,           { 128, 128, 128, 255 } }
};

Color palette_find_language_color(const char* lang_name) {
    if (!lang_name || !*lang_name) return hex_color(56, 189, 248); /* Default sapphire/cyan */

    /* If user passed hex string e.g. "#3572A5" or "3572A5" */
    const char* p = lang_name;
    if (*p == '#') p++;
    if (strlen(p) == 6) {
        int r, g, b;
        if (sscanf(p, "%02x%02x%02x", &r, &g, &b) == 3) {
            return hex_color((uint8_t)r, (uint8_t)g, (uint8_t)b);
        }
    }

    /* Lowercase lookup */
    char lower[64];
    int len = (int)strlen(lang_name);
    if (len >= 64) len = 63;
    for (int i = 0; i < len; i++) {
        lower[i] = (char)tolower((unsigned char)lang_name[i]);
    }
    lower[len] = '\0';

    for (int i = 0; GITHUB_LANG_TABLE[i].name != NULL; i++) {
        if (strcmp(lower, GITHUB_LANG_TABLE[i].name) == 0) {
            return GITHUB_LANG_TABLE[i].color;
        }
    }

    /* Fallback hash-based procedural color if language unknown */
    uint32_t h = 2166136261u;
    for (int i = 0; lower[i]; i++) {
        h = (h ^ (uint8_t)lower[i]) * 16777619u;
    }
    uint8_t hr = 60 + (h & 0x7F);
    uint8_t hg = 60 + ((h >> 8) & 0x7F);
    uint8_t hb = 60 + ((h >> 16) & 0x7F);
    return hex_color(hr, hg, hb);
}
