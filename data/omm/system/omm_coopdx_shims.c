#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#include "audio/external.h"
#include "game/object_helpers.h"

//
// Symbols that OMM expects coopdx (or its own removed modules) to provide.
// Compiled only in the Android build (data/omm is not part of the PC build).
//

// Audio: coopdx exposes play_music(); OMM code calls play_sequence().
void play_sequence(u8 player, u8 seqId, u16 fadeTimer) {
    play_music(player, seqId, fadeTimer);
}

// Model textures: OMM asks whether a texture has a palette; coopdx textures
// are always built without palettes here.
bool gfx_texture_has_palette(const char *texname) {
    (void) texname;
    return false;
}

// New camera config globals written by omm_camera_init_config().
s16 newcam_sensitivityX = 0;
s16 newcam_sensitivityY = 0;
s16 newcam_analogue = 0;
s16 newcam_panlevel = 0;
s16 newcam_degrade = 0;

// Surface hash map used by the OMM camera to gather wall surfaces.
// OMM's surface_load.c (which builds it) is excluded from the Android build,
// so provide an empty map: the camera falls back to ray tests only.
OmmSurfaceHashMap gOmmSurfaces[2] = {0};

// HUD: star select coin counter.
void omm_render_hud_coins(s16 x, s16 y, s16 w, u8 alpha, s32 coins) {
    OMM_RENDER_DEFINE_GLYPH_SIZE(w);
    omm_render_number_hud(x, y, alpha, coins, 3, true, false);
}

//
// Capture behaviors: OMM's behavior override files are never included in the
// coopdx build, so re-implement the two functions the capture modules use.
//

#define o gCurrentObject

static const f32 sOmmDorrieCircles[] = {
    -3550, 3550, 4000, // Bounds
    -3550, 3550, 1100, // Central island
      500, 2900, 1100, // Star 6
    -1250, 5885,  500, // Metal cap
      -60, 7000, 1600,
};

void bhv_dorrie_update_pos(struct Object *obj, f32 radius) {
    for (u8 done = FALSE; !done;) {
        Vec3f pos = { obj->oPosX, obj->oPosY, obj->oPosZ };

        // Keep Dorrie in bounds
        const f32 *bounds = sOmmDorrieCircles;
        f32 dx = obj->oPosX - bounds[0];
        f32 dz = obj->oPosZ - bounds[1];
        f32 dist = sqrtf(sqr_f(dx) + sqr_f(dz));
        if (dist + radius > bounds[2]) {
            obj->oPosX = bounds[0] + (bounds[2] - radius) * (dx / dist);
            obj->oPosZ = bounds[1] + (bounds[2] - radius) * (dz / dist);
        }

        // Move Dorrie out of circles
        for (s32 i = 3; i != array_length(sOmmDorrieCircles); i += 3) {
            const f32 *circle = sOmmDorrieCircles + i;
            f32 dx2 = obj->oPosX - circle[0];
            f32 dz2 = obj->oPosZ - circle[1];
            f32 dist2 = sqrtf(sqr_f(dx2) + sqr_f(dz2));
            if (dist2 < circle[2] + radius) {
                obj->oPosX = circle[0] + (circle[2] + radius) * (dx2 / dist2);
                obj->oPosZ = circle[1] + (circle[2] + radius) * (dz2 / dist2);
            }
        }

        // Has Dorrie moved?
        done = mem_eq(pos, &obj->oPosX, sizeof(Vec3f));
    }
    o->oPosY = o->oHomeY + o->oDorrieOffsetY;
}

void omm_monty_mole_spawn_dirt_particles(s8 offsetY, s8 velYBase) {
    static struct SpawnParticlesInfo sOmmMontyMoleRiseFromGroundParticles = { 0, 3, MODEL_SAND_DUST, 0, 4, 4, 10, 15, -4, 0, 10, 7 };
    sOmmMontyMoleRiseFromGroundParticles.offsetY = offsetY;
    sOmmMontyMoleRiseFromGroundParticles.velYBase = velYBase;
    cur_obj_spawn_particles(&sOmmMontyMoleRiseFromGroundParticles);
}

//
// Second round of symbols missing from the coopdx build (defs were in
// modules excluded from the Android build).
//

#include "behavior_commands.h"
#include "level_commands.h"

// Interactions: coopdx has no interact_unknown_08 handler or sInvulnerable.
u32 interact_unknown_08(struct MarioState *m, u32 interactType, struct Object *o) {
    (void) m; (void) interactType; (void) o;
    return 0;
}

s16 sInvulnerable = 0;

// Peachy room trigger behavior (defined in the excluded omm_level_peachy.c).
const BehaviorScript bhvOmmPeachyRoomTrigger[] = {
    OBJ_TYPE_DEFAULT,
    BHV_BREAK(),
};

// Sparkly star sparkle effect (defined in the excluded peachy modules).
void bhv_omm_sparkly_grand_star_spawn_sparkles() {
}

// Level palette color callback (coopdx gfx has no level palettes).
void gfx_texture_set_level_palette_color(f32 r, f32 g, f32 b) {
    (void) r; (void) g; (void) b;
}

// Frame interpolation global used by several OMM modules.
bool gFrameInterpolation = false;

// Palette editor level (defined in the excluded omm_palette_editor.c).
const LevelScript omm_level_palette_editor[] = {
    EXIT(),
};

// Options menu callbacks (defined in the excluded dialog/menu modules).
void omm_opt_return_to_previous_menu(struct Option *opt, s32 arg) {
    (void) opt; (void) arg;
}

void omm_opt_init_main_menu(void) {
}