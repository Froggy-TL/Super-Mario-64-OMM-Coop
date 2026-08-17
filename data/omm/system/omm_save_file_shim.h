#ifndef OMM_SAVE_FILE_SHIM_H
#define OMM_SAVE_FILE_SHIM_H

// OMM save file compatibility layer over the coopdx save system.
// Implements the OMM omm_save_file_* API using coopdx's src/game/save_file.c.

#include <PR/ultratypes.h>
#include "types.h"
#include "course_table.h"

#define NUM_SAVE_MODES (2)
#define NUM_KEYS (10)
#define NUM_WARIO_COINS (6)

#define OMM_SAVE_FILE_SECTION_GAME_DATA     "[%s:%c%d]"
#define OMM_SAVE_FILE_SECTION_SPARKLY_STARS "[sparkly_stars]"
#define OMM_SAVE_FILE_SECTION_STATS         "[stats]"
#define OMM_SAVE_FILE_SECTION_SECRETS       "[secrets]"
#define OMM_SAVE_FILE_SECTION_MARIO_COLORS  "[mario_colors]"
#define OMM_SAVE_FILE_SECTION_PEACH_COLORS  "[peach_colors]"

#define OMM_SAVE_FILE_WRITE_BUFFER_LENGTH (0x20000)

extern char *gOmmSaveFileWriteBuffer;
extern s32 gOmmSaveFileWrittenLength;

bool omm_save_file_exists                (s32 fileIndex, s32 modeIndex);
u32  omm_save_file_get_flags             (s32 fileIndex, s32 modeIndex);
u32  omm_save_file_get_star_flags        (s32 fileIndex, s32 modeIndex, s32 courseIndex);
u32  omm_save_file_get_cannon_unlocked   (s32 fileIndex, s32 modeIndex, s32 courseIndex);
s32  omm_save_file_get_course_star_count (s32 fileIndex, s32 modeIndex, s32 courseIndex);
s32  omm_save_file_get_course_coin_score (s32 fileIndex, s32 modeIndex, s32 courseIndex);
s32  omm_save_file_get_total_star_count  (s32 fileIndex, s32 modeIndex);
s32  omm_save_file_get_total_coin_score  (s32 fileIndex, s32 modeIndex);
s32  omm_save_file_get_last_file_index   (void);
s32  omm_save_file_get_last_mode_index   (void);
s32  omm_save_file_get_last_course_num   (s32 fileIndex, s32 modeIndex);
u64  omm_save_file_get_capture_flags     (s32 fileIndex, s32 modeIndex);
s32  omm_save_file_get_capture_count     (s32 fileIndex, s32 modeIndex);
u32  omm_save_file_get_taken_luigi_key   (s32 fileIndex, s32 modeIndex, s32 keyIndex);
u32  omm_save_file_get_taken_wario_coin  (s32 fileIndex, s32 modeIndex, s32 coinIndex);
s32  omm_save_file_get_luigi_keys_count  (s32 fileIndex, s32 modeIndex);
s32  omm_save_file_get_wario_coins_count (s32 fileIndex, s32 modeIndex);
bool omm_save_file_are_all_captures_registered_sm64(void);

void omm_save_file_do_save               (void);
void omm_save_file_load_all              (void);
void omm_save_file_set_flags             (s32 fileIndex, s32 modeIndex, u32 flags);
void omm_save_file_set_star_flags        (s32 fileIndex, s32 modeIndex, s32 courseIndex, u32 starFlags);
void omm_save_file_set_cannon_unlocked   (s32 fileIndex, s32 modeIndex, s32 courseIndex);
void omm_save_file_set_last_course       (s32 fileIndex, s32 modeIndex, s32 courseIndex);
void omm_save_file_register_capture      (s32 fileIndex, s32 modeIndex, u64 captureFlag);
void omm_save_file_set_taken_luigi_key   (s32 fileIndex, s32 modeIndex, s32 keyIndex);
void omm_save_file_set_taken_wario_coin  (s32 fileIndex, s32 modeIndex, s32 coinIndex);
void omm_save_file_collect_star_or_key   (s32 fileIndex, s32 modeIndex, s32 levelIndex, s32 starIndex, s32 numCoins);
void omm_save_file_clear_flags           (s32 fileIndex, s32 modeIndex, u32 flags);
void omm_save_file_clear_star_flags      (s32 fileIndex, s32 modeIndex, s32 courseIndex, u32 starFlags);
void omm_save_file_erase                 (s32 fileIndex, s32 modeIndex);
void omm_save_file_copy                  (s32 fileIndex, s32 modeIndex, s32 destIndex);

struct WarpNode;
bool warp_checkpoint_check               (struct WarpNode *warpNode, s32 actIndex);
void warp_checkpoint_check_if_should_set (struct WarpNode *warpNode, s32 courseIndex, s32 actIndex);
void warp_checkpoint_disable             (void);

struct OmmStats;
struct OmmStats *omm_save_file_get_stats (s32 fileIndex, s32 modeIndex);

#define omm_save_file_write_buffer(...) { \
    if (gOmmSaveFileWrittenLength < OMM_SAVE_FILE_WRITE_BUFFER_LENGTH) { \
        s32 written = snprintf(gOmmSaveFileWriteBuffer, OMM_SAVE_FILE_WRITE_BUFFER_LENGTH - gOmmSaveFileWrittenLength, __VA_ARGS__); \
        gOmmSaveFileWriteBuffer += written; \
        gOmmSaveFileWrittenLength += written; \
    } \
}

#endif // OMM_SAVE_FILE_SHIM_H
