#ifndef REPO_H
#define REPO_H

#include "traits.h"
#include "palette.h"
#include "pixel_grid.h"

/* Growth Stages for a GitHub Repository Betta */
typedef enum {
    GROWTH_FRY = 0,      /* Baby Fry (1 - 10 commits): Small, cute, compact, short bud fins */
    GROWTH_JUVENILE,     /* Juvenile (11 - 50 commits): Body elongating, fins expanding */
    GROWTH_ADULT,        /* Mature Adult (51 - 250 commits): Full spread flowing fins, bold scales */
    GROWTH_ELDER,        /* Master Elder (251 - 1000 commits): Grand regal proportions, sweeping cascades */
    GROWTH_LEVIATHAN     /* Mythic Leviathan (1000+ commits): Towering fins, ornate ruffles, titan body */
} GrowthStage;

/* Language Share Breakdown in a GitHub Repository */
typedef struct {
    char  name[32];      /* Language name (e.g. "C", "Python", "Rust", "TypeScript", or #RRGGBB) */
    float percentage;    /* Proportional weight (e.g. 70.0 for 70%) */
    Color color;         /* Canonical GitHub Linguist color */
} RepoLanguage;

/* GitHub Repository Profile */
typedef struct {
    char         repo_name[128];       /* Seed text: repository name (e.g. "torvalds/linux", "connor/git-pet") */
    int          commits;             /* Total commit count (controls growth) */
    int          language_count;      /* Number of parsed languages */
    RepoLanguage languages[8];        /* Up to 8 languages */
} RepoProfile;

/* Parse a repository profile from name, commit count, and language spec string (e.g. "C:70,Python:25,Shell:5") */
RepoProfile repo_profile_parse(const char* repo_name, int commits, const char* lang_spec);

/* Determine growth stage from commit count */
GrowthStage repo_growth_stage(int commits);

/* Human-readable growth stage name */
const char* repo_growth_stage_name(GrowthStage stage);

/* Continuous growth factor in range [0.0, 1.0] */
float repo_growth_factor(int commits);

/* Create the base genetic traits for a repository from its name seed */
BettaTraits betta_traits_from_repo(const RepoProfile* repo);

/* Growth Function: Transforms fish traits based on commit count!
 * Causes the fish to grow continuously from a tiny baby fry to a grand leviathan. */
void betta_grow(BettaTraits* traits, int commits);

/* Multi-Color Palette Generator:
 * Derives a rich multi-colored palette from repository languages (Body = Lang 1, Fins = Lang 2, Accents = Lang 3). */
void palette_from_repo(PixelGrid* g, const RepoProfile* repo);

#endif /* REPO_H */
