#include "repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

GrowthStage repo_growth_stage(int commits) {
    if (commits < 12)   return GROWTH_FRY;
    if (commits < 60)   return GROWTH_JUVENILE;
    if (commits < 250)  return GROWTH_ADULT;
    if (commits < 1000) return GROWTH_ELDER;
    return GROWTH_LEVIATHAN;
}

const char* repo_growth_stage_name(GrowthStage stage) {
    switch (stage) {
        case GROWTH_FRY:       return "Baby Fry (Level 1)";
        case GROWTH_JUVENILE:  return "Juvenile (Level 2)";
        case GROWTH_ADULT:     return "Mature Adult (Level 3)";
        case GROWTH_ELDER:     return "Master Elder (Level 4)";
        case GROWTH_LEVIATHAN: return "Mythic Leviathan (Level 5)";
        default:               return "Active";
    }
}

float repo_growth_factor(int commits) {
    if (commits <= 1) return 0.03f;
    /* Smooth continuous S-curve from baby fry (g=0.03) to mythic leviathan (g=0.96) */
    float c = (float)commits;
    float g = 1.0f - (1.0f / (1.0f + powf(c / 65.0f, 0.85f)));
    if (g < 0.03f) g = 0.03f;
    if (g > 0.98f) g = 0.98f;
    return g;
}

RepoProfile repo_profile_parse(const char* repo_name, int commits, const char* lang_spec) {
    RepoProfile p;
    memset(&p, 0, sizeof(p));

    const char* actual_name = (repo_name && *repo_name) ? repo_name : "user/git-betta";
    strncpy(p.repo_name, actual_name, sizeof(p.repo_name) - 1);
    p.commits = (commits >= 1) ? commits : 1;

    if (!lang_spec || !*lang_spec) {
        /* Default single language based on repo name */
        p.language_count = 1;
        strncpy(p.languages[0].name, "C", sizeof(p.languages[0].name) - 1);
        p.languages[0].percentage = 100.0f;
        p.languages[0].color = palette_find_language_color("C");
        return p;
    }

    /* Parse comma-separated list like "C:70,Python:25,Shell:5" or "rust" */
    char buf[256];
    strncpy(buf, lang_spec, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* token = strtok(buf, ",;");
    while (token && p.language_count < 8) {
        while (*token == ' ') token++;
        if (!*token) {
            token = strtok(NULL, ",;");
            continue;
        }

        char* colon = strchr(token, ':');
        float pct = 1.0f;
        if (colon) {
            *colon = '\0';
            pct = (float)atof(colon + 1);
            if (pct <= 0.0f) pct = 1.0f;
        }

        /* Trim language name */
        char name[32];
        strncpy(name, token, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        char* end = name + strlen(name) - 1;
        while (end > name && *end == ' ') *end-- = '\0';

        snprintf(p.languages[p.language_count].name, sizeof(p.languages[p.language_count].name), "%s", name);
        p.languages[p.language_count].percentage = pct;
        p.languages[p.language_count].color = palette_find_language_color(name);
        p.language_count++;

        token = strtok(NULL, ",;");
    }

    if (p.language_count == 0) {
        p.language_count = 1;
        strncpy(p.languages[0].name, "C", sizeof(p.languages[0].name) - 1);
        p.languages[0].percentage = 100.0f;
        p.languages[0].color = palette_find_language_color("C");
    }

    /* Sort languages descending by percentage */
    for (int i = 0; i < p.language_count - 1; i++) {
        for (int j = i + 1; j < p.language_count; j++) {
            if (p.languages[i].percentage < p.languages[j].percentage) {
                RepoLanguage tmp = p.languages[i];
                p.languages[i] = p.languages[j];
                p.languages[j] = tmp;
            }
        }
    }

    return p;
}

BettaTraits betta_traits_from_repo(const RepoProfile* repo) {
    if (!repo) return betta_traits_from_seed("git-betta");
    BettaTraits traits = betta_traits_from_seed(repo->repo_name);
    betta_grow(&traits, repo->commits);
    return traits;
}

/* Growth Function: Causes the fish to grow continuously from baby fry to grand leviathan */
void betta_grow(BettaTraits* traits, int commits) {
    if (!traits) return;

    float g = repo_growth_factor(commits);

    /* 1. Body Length & Posture Scaling */
    /* Fry starts compact (42-48 px), growing to full adult (64-72 px), up to Leviathan (78-84 px) */
    float base_len = (float)traits->body_length;
    traits->body_length = (int)roundf(base_len * (0.64f + 0.44f * g));

    /* Fry has a cute chubby baby belly; adults develop sleek muscular curves */
    if (g < 0.30f) {
        traits->belly_depth *= (1.25f - 0.5f * g);
        traits->peduncle_depth *= (0.75f + 0.5f * g);
    } else {
        traits->peduncle_depth *= (0.85f + 0.30f * g);
    }

    /* 2. Caudal Fin Growth */
    /* Baby fry has a cute short bud; mature adult has full billowing plume */
    traits->caudal_length *= (0.42f + 0.70f * g);
    traits->caudal_spread *= (0.55f + 0.52f * g);
    traits->caudal_rays = (int)roundf((float)traits->caudal_rays * (0.50f + 0.55f * g));
    if (traits->caudal_rays < 14) traits->caudal_rays = 14;

    /* Spades and Clefts emerge as the fish matures */
    traits->spade_sharpness *= (g > 0.20f ? (g - 0.20f) / 0.80f : 0.0f);
    traits->cleft_depth     *= (g > 0.15f ? (g - 0.15f) / 0.85f : 0.0f);

    /* Rosetail petal ruffles only unfurl in adulthood */
    traits->ruffle_amp *= (g * g);

    /* Crowntail spikes emerge and sharpen as it grows */
    traits->spike_protrusion *= (0.25f + 0.75f * g);

    /* Gravity droop waterfall increases as fins become long and heavy */
    traits->gravity_droop *= (0.20f + 0.85f * g);

    /* 3. Dorsal and Anal Fin Growth */
    traits->dorsal_height *= (0.38f + 0.72f * g);
    traits->anal_depth    *= (0.40f + 0.70f * g);

    /* 4. Pectoral & Pelvic Growth */
    traits->pectoral_width  *= (0.50f + 0.60f * g);
    traits->pectoral_height *= (0.50f + 0.60f * g);
    traits->ventral_length  *= (0.40f + 0.65f * g);

    /* 5. Scales & Facial Maturation */
    if (g < 0.25f) {
        traits->scale_step = 3;       /* Fine baby scales */
        traits->scale_density = 1;
        traits->eye_style = EYE_STYLE_WIDE; /* Large curious baby eyes */
    } else if (g > 0.75f) {
        traits->scale_density = 3;    /* Bold armored relief in elders */
    }
}

void palette_from_repo(PixelGrid* g, const RepoProfile* repo) {
    if (!g || !repo) return;

    if (repo->language_count == 0) {
        palette_load_theme(g, THEME_SAPPHIRE);
        return;
    }

    Color primary   = repo->languages[0].color;
    Color secondary = (repo->language_count > 1) ? repo->languages[1].color :
                      (Color){ (uint8_t)(primary.r * 0.7f + 60), (uint8_t)(primary.g * 0.7f + 60), (uint8_t)(primary.b * 0.7f + 60), 255 };
    Color accent    = (repo->language_count > 2) ? repo->languages[2].color :
                      (Color){ (uint8_t)(255 - primary.r / 2), (uint8_t)(255 - primary.g / 2), (uint8_t)(255 - primary.b / 2), 255 };

    palette_from_colors(g, primary, secondary, accent);
}
