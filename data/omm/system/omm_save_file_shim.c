#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#include "game/save_file.h"
#include "data/omm/system/omm_save_file_shim.h"

// OMM save file compatibility layer over the coopdx save system.

extern struct SaveBuffer gSaveBuffer;
extern s8 gSaveFileModified;

#define OMM_INVALID_COURSE_STAR_INDEX(_ci) ((u32)(_ci) >= COURSE_COUNT)

static u64 sOmmCaptureFlags[NUM_SAVE_FILES][NUM_SAVE_MODES] = { 0 };
static u32 sOmmTakenKeys[NUM_SAVE_FILES][NUM_SAVE_MODES] = { 0 };
static u32 sOmmTakenCoins[NUM_SAVE_FILES][NUM_SAVE_MODES] = { 0 };

char *gOmmSaveFileWriteBuffer = NULL;
s32 gOmmSaveFileWrittenLength = 0;

static void omm_save_file_reset_stats(OmmStats *stats) {
    bzero(stats, sizeof(OmmStats));
}

struct OmmStats *omm_save_file_get_stats(s32 fileIndex, s32 modeIndex) {
    return gOmmData->stats;
}

static s32 omm_save_file_clamp_index(s32 fileIndex) {
    if (fileIndex < 0) { return 0; }
    if (fileIndex >= NUM_SAVE_FILES) { return NUM_SAVE_FILES - 1; }
    return fileIndex;
}

bool omm_save_file_exists(s32 fileIndex, s32 modeIndex) {
    return save_file_exists(omm_save_file_clamp_index(fileIndex)) == 1;
}

u32 omm_save_file_get_flags(s32 fileIndex, s32 modeIndex) {
    if (fileIndex >= 0 && fileIndex < NUM_SAVE_FILES) {
        return gSaveBuffer.files[fileIndex][0].flags;
    }
    return 0;
}

u32 omm_save_file_get_star_flags(s32 fileIndex, s32 modeIndex, s32 courseIndex) {
    return save_file_get_star_flags(omm_save_file_clamp_index(fileIndex), courseIndex);
}

u32 omm_save_file_get_cannon_unlocked(s32 fileIndex, s32 modeIndex, s32 courseIndex) {
    return save_file_is_cannon_unlocked(omm_save_file_clamp_index(fileIndex), courseIndex);
}

s32 omm_save_file_get_course_star_count(s32 fileIndex, s32 modeIndex, s32 courseIndex) {
    return save_file_get_course_star_count(omm_save_file_clamp_index(fileIndex), courseIndex);
}

s32 omm_save_file_get_course_coin_score(s32 fileIndex, s32 modeIndex, s32 courseIndex) {
    return save_file_get_course_coin_score(omm_save_file_clamp_index(fileIndex), courseIndex);
}

s32 omm_save_file_get_total_star_count(s32 fileIndex, s32 modeIndex) {
    return save_file_get_total_star_count(omm_save_file_clamp_index(fileIndex), COURSE_MIN, COURSE_COUNT - 1);
}

s32 omm_save_file_get_total_coin_score(s32 fileIndex, s32 modeIndex) {
    s32 total = 0;
    for (s32 i = COURSE_MIN; i < COURSE_STAGES_COUNT; i++) {
        total += save_file_get_course_coin_score(omm_save_file_clamp_index(fileIndex), i);
    }
    return total;
}

s32 omm_save_file_get_last_file_index(void) {
    return gCurrSaveFileNum - 1;
}

s32 omm_save_file_get_last_mode_index(void) {
    return OMM_GAME_MODE;
}

s32 omm_save_file_get_last_course_num(s32 fileIndex, s32 modeIndex) {
    return gLastCompletedCourseNum;
}

u64 omm_save_file_get_capture_flags(s32 fileIndex, s32 modeIndex) {
    return sOmmCaptureFlags[omm_save_file_clamp_index(fileIndex)][modeIndex & 1];
}

s32 omm_save_file_get_capture_count(s32 fileIndex, s32 modeIndex) {
    u64 flags = omm_save_file_get_capture_flags(fileIndex, modeIndex);
    s32 count = 0;
    while (flags) {
        flags &= flags - 1;
        count++;
    }
    return count;
}

u32 omm_save_file_get_taken_luigi_key(s32 fileIndex, s32 modeIndex, s32 keyIndex) {
    if (keyIndex < 0 || keyIndex >= NUM_KEYS) { return 0; }
    return (sOmmTakenKeys[omm_save_file_clamp_index(fileIndex)][modeIndex & 1] >> keyIndex) & 1;
}

u32 omm_save_file_get_taken_wario_coin(s32 fileIndex, s32 modeIndex, s32 coinIndex) {
    if (coinIndex < 0 || coinIndex >= NUM_WARIO_COINS) { return 0; }
    return (sOmmTakenCoins[omm_save_file_clamp_index(fileIndex)][modeIndex & 1] >> coinIndex) & 1;
}

s32 omm_save_file_get_luigi_keys_count(s32 fileIndex, s32 modeIndex) {
    s32 count = 0;
    for (s32 i = 0; i < NUM_KEYS; i++) {
        count += omm_save_file_get_taken_luigi_key(fileIndex, modeIndex, i);
    }
    return count;
}

s32 omm_save_file_get_wario_coins_count(s32 fileIndex, s32 modeIndex) {
    s32 count = 0;
    for (s32 i = 0; i < NUM_WARIO_COINS; i++) {
        count += omm_save_file_get_taken_wario_coin(fileIndex, modeIndex, i);
    }
    return count;
}

bool omm_save_file_are_all_captures_registered_sm64(void) {
    s32 fileIndex = omm_save_file_get_last_file_index();
    return omm_save_file_get_capture_count(fileIndex, OMM_GAME_MODE) >= 0x20;
}

void omm_save_file_do_save(void) {
    save_file_do_save(gCurrSaveFileNum - 1, TRUE);
}

void omm_save_file_load_all(void) {
    save_file_load_all(FALSE);
}

void omm_save_file_set_flags(s32 fileIndex, s32 modeIndex, u32 flags) {
    fileIndex = omm_save_file_clamp_index(fileIndex);
    gSaveBuffer.files[fileIndex][0].flags |= flags;
    gSaveFileModified = TRUE;
}

void omm_save_file_set_star_flags(s32 fileIndex, s32 modeIndex, s32 courseIndex, u32 starFlags) {
    save_file_set_star_flags(omm_save_file_clamp_index(fileIndex), courseIndex, starFlags);
}

void omm_save_file_set_cannon_unlocked(s32 fileIndex, s32 modeIndex, s32 courseIndex) {
    fileIndex = omm_save_file_clamp_index(fileIndex);
    if (OMM_INVALID_COURSE_STAR_INDEX(courseIndex)) { return; }
    gSaveBuffer.files[fileIndex][0].courseStars[courseIndex] |= 0x80;
    gSaveBuffer.files[fileIndex][0].flags |= SAVE_FLAG_FILE_EXISTS;
    gSaveFileModified = TRUE;
}

void omm_save_file_set_last_course(s32 fileIndex, s32 modeIndex, s32 courseIndex) {
    gLastCompletedCourseNum = courseIndex;
}

void omm_save_file_register_capture(s32 fileIndex, s32 modeIndex, u64 captureFlag) {
    fileIndex = omm_save_file_clamp_index(fileIndex);
    sOmmCaptureFlags[fileIndex][modeIndex & 1] |= captureFlag;
}

void omm_save_file_set_taken_luigi_key(s32 fileIndex, s32 modeIndex, s32 keyIndex) {
    if (keyIndex < 0 || keyIndex >= NUM_KEYS) { return; }
    fileIndex = omm_save_file_clamp_index(fileIndex);
    sOmmTakenKeys[fileIndex][modeIndex & 1] |= (1u << keyIndex);
}

void omm_save_file_set_taken_wario_coin(s32 fileIndex, s32 modeIndex, s32 coinIndex) {
    if (coinIndex < 0 || coinIndex >= NUM_WARIO_COINS) { return; }
    fileIndex = omm_save_file_clamp_index(fileIndex);
    sOmmTakenCoins[fileIndex][modeIndex & 1] |= (1u << coinIndex);
}

void omm_save_file_collect_star_or_key(s32 fileIndex, s32 modeIndex, s32 levelIndex, s32 starIndex, s32 numCoins) {
    s32 courseIndex = gLevelToCourseNumTable[levelIndex];
    save_file_collect_star_or_key(numCoins, starIndex, FALSE);
    (void) courseIndex;
}

void omm_save_file_clear_flags(s32 fileIndex, s32 modeIndex, u32 flags) {
    fileIndex = omm_save_file_clamp_index(fileIndex);
    gSaveBuffer.files[fileIndex][0].flags &= ~flags;
    gSaveFileModified = TRUE;
}

void omm_save_file_clear_star_flags(s32 fileIndex, s32 modeIndex, s32 courseIndex, u32 starFlags) {
    save_file_remove_star_flags(omm_save_file_clamp_index(fileIndex), courseIndex, starFlags);
}

void omm_save_file_erase(s32 fileIndex, s32 modeIndex) {
    fileIndex = omm_save_file_clamp_index(fileIndex);
    save_file_erase(fileIndex);
    sOmmCaptureFlags[fileIndex][modeIndex & 1] = 0;
    sOmmTakenKeys[fileIndex][modeIndex & 1] = 0;
    sOmmTakenCoins[fileIndex][modeIndex & 1] = 0;
    omm_save_file_reset_stats(gOmmData->stats);
}

void omm_save_file_copy(s32 fileIndex, s32 modeIndex, s32 destIndex) {
    fileIndex = omm_save_file_clamp_index(fileIndex);
    destIndex = omm_save_file_clamp_index(destIndex);
    save_file_copy(fileIndex, destIndex);
    sOmmCaptureFlags[destIndex][modeIndex & 1] = sOmmCaptureFlags[fileIndex][modeIndex & 1];
    sOmmTakenKeys[destIndex][modeIndex & 1] = sOmmTakenKeys[fileIndex][modeIndex & 1];
    sOmmTakenCoins[destIndex][modeIndex & 1] = sOmmTakenCoins[fileIndex][modeIndex & 1];
}

bool warp_checkpoint_check(struct WarpNode *warpNode, s32 actIndex) {
    return check_warp_checkpoint(warpNode) == TRUE;
}

void warp_checkpoint_check_if_should_set(struct WarpNode *warpNode, s32 courseIndex, s32 actIndex) {
    check_if_should_set_warp_checkpoint(warpNode);
}

void warp_checkpoint_disable(void) {
    disable_warp_checkpoint();
}
