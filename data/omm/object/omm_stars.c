#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#include "data/omm/omm_constants.h"

// The original OMM star system read star data from level scripts and stored
// flags in a dedicated save buffer (sOmmStarsFlags). In coopdx, star flags are
// handled by the engine's save_file module, so this shim delegates to it.
//
// coopdx save_file stores one u8 per course (courseStars[]), using courseNum
// as the index. OMM used levelNum + area bitmasks; get_level_flags() folds the
// per-area flags into a single per-level bitmask, which is what coopdx already
// stores per course. Bowser/secret levels map to the same course in both.

u8 sOmmStarsFlags = 0;

OMM_ROUTINE_LEVEL_ENTRY(omm_stars_init) {
    sOmmStarsFlags = 0;
}

static u8 omm_stars_get_course_flags_raw(s32 courseNum) {
    if (courseNum < 0) { return 0; }
    return (u8) (save_file_get_star_flags(gCurrSaveFileNum - 1, courseNum) & 0x7F);
}

u8 omm_stars_get_flags() {
    return sOmmStarsFlags;
}

u8 omm_stars_get_level_flags(s32 levelNum, MODE_INDEX s32 modeIndex) {
    (void) modeIndex;
    return omm_stars_get_course_flags_raw(get_level_course_num(levelNum));
}

u8 omm_stars_get_course_flags(s32 courseNum, MODE_INDEX s32 modeIndex) {
    (void) modeIndex;
    return omm_stars_get_course_flags_raw(courseNum);
}

s32 omm_stars_get_total_star_count(MODE_INDEX s32 modeIndex) {
    (void) modeIndex;
    s32 starCount = 0;
    for (s32 course = COURSE_MIN; course <= COURSE_MAX; ++course) {
        u8 flags = omm_stars_get_course_flags_raw(course);
        for (s32 i = 0; i != 7; ++i) {
            starCount += ((flags >> i) & 1);
        }
    }
    return starCount;
}

u32 omm_stars_get_color(s32 levelNum, MODE_INDEX s32 modeIndex) {
    (void) levelNum;
    (void) modeIndex;
    return 0x00FFFFFF;
}

bool omm_stars_is_collected(s32 starIndex) {
    return OMM_STARS_NON_STOP && ((sOmmStarsFlags >> starIndex) & 1);
}

bool omm_stars_all_collected(s32 levelNum, MODE_INDEX s32 modeIndex) {
    (void) modeIndex;
    if (OMM_LEVEL_IS_BOWSER_FIGHT(levelNum)) { return true; }
    u8 flags = omm_stars_get_level_flags(levelNum, 0);
    return flags != 0 && (sOmmStarsFlags & flags) == flags;
}

void omm_stars_set_flags(u8 starFlags) {
    sOmmStarsFlags |= starFlags;
}

OMM_ROUTINE_UPDATE(omm_stars_update) {
    // No-op: coopdx engine manages collected stars and collected object cleanup.
}
