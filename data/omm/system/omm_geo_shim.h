#ifndef OMM_GEO_SHIM_H
#define OMM_GEO_SHIM_H

#include "game/game_init.h"
#include "src/engine/geo_layout.h"
#include "src/pc/fs/fs.h"
#include "data/omm/omm_macros_coopdx.h"

// Level script command/status macros from the original OMM port. coopdx keeps
// the same vanilla format but does not expose these constants.
#ifndef LEVEL_SCRIPT_CONTINUE
#define LEVEL_SCRIPT_CONTINUE               (0)
#define LEVEL_SCRIPT_NEXT                   (1)
#define LEVEL_SCRIPT_RETURN                 (2)
#define LEVEL_SCRIPT_STOP                   (3)
#define LEVEL_SCRIPT_JUMP                   (4)

#define LEVEL_CMD_EXECUTE                   (0x00)
#define LEVEL_CMD_EXIT_AND_EXECUTE          (0x01)
#define LEVEL_CMD_EXIT                      (0x02)
#define LEVEL_CMD_SLEEP                     (0x03)
#define LEVEL_CMD_SLEEP_BEFORE_EXIT         (0x04)
#define LEVEL_CMD_JUMP                      (0x05)
#define LEVEL_CMD_JUMP_LINK                 (0x06)
#define LEVEL_CMD_RETURN                    (0x07)
#define LEVEL_CMD_JUMP_LINK_PUSH_ARG        (0x08)
#define LEVEL_CMD_JUMP_N_TIMES              (0x09)
#define LEVEL_CMD_LOOP_BEGIN                (0x0A)
#define LEVEL_CMD_LOOP_UNTIL                (0x0B)
#define LEVEL_CMD_JUMP_IF                   (0x0C)
#define LEVEL_CMD_JUMP_LINK_IF              (0x0D)
#define LEVEL_CMD_SKIP_IF                   (0x0E)
#define LEVEL_CMD_SKIP                      (0x0F)
#define LEVEL_CMD_SKIP_NOP                  (0x10)
#define LEVEL_CMD_CALL                      (0x11)
#define LEVEL_CMD_CALL_LOOP                 (0x12)
#define LEVEL_CMD_SET_REG                   (0x13)
#define LEVEL_CMD_PUSH_POOL                 (0x14)
#define LEVEL_CMD_POP_POOL                  (0x15)
#define LEVEL_CMD_FIXED_LOAD                (0x16)
#define LEVEL_CMD_LOAD_RAW                  (0x17)
#define LEVEL_CMD_LOAD_MIO0                 (0x18)
#define LEVEL_CMD_LOAD_MARIO_HEAD           (0x19)
#define LEVEL_CMD_LOAD_MIO0_TEXTURE         (0x1A)
#define LEVEL_CMD_INIT_LEVEL                (0x1B)
#define LEVEL_CMD_CLEAR_LEVEL               (0x1C)
#define LEVEL_CMD_ALLOC_LEVEL_POOL          (0x1D)
#define LEVEL_CMD_FREE_LEVEL_POOL           (0x1E)
#define LEVEL_CMD_AREA                      (0x1F)
#define LEVEL_CMD_END_AREA                  (0x20)
#define LEVEL_CMD_LOAD_MODEL_FROM_DL        (0x21)
#define LEVEL_CMD_LOAD_MODEL_FROM_GEO       (0x22)
#define LEVEL_CMD_CMD23                     (0x23)
#define LEVEL_CMD_OBJECT_WITH_ACTS          (0x24)
#define LEVEL_CMD_MARIO                     (0x25)
#define LEVEL_CMD_WARP_NODE                 (0x26)
#define LEVEL_CMD_PAINTING_WARP_NODE        (0x27)
#define LEVEL_CMD_INSTANT_WARP              (0x28)
#define LEVEL_CMD_LOAD_AREA                 (0x29)
#define LEVEL_CMD_CMD2A                     (0x2A)
#define LEVEL_CMD_MARIO_POS                 (0x2B)
#define LEVEL_CMD_CMD2C                     (0x2C)
#define LEVEL_CMD_CMD2D                     (0x2D)
#define LEVEL_CMD_TERRAIN                   (0x2E)
#define LEVEL_CMD_ROOMS                     (0x2F)
#define LEVEL_CMD_SHOW_DIALOG               (0x30)
#define LEVEL_CMD_TERRAIN_TYPE              (0x31)
#define LEVEL_CMD_NOP                       (0x32)
#define LEVEL_CMD_TRANSITION                (0x33)
#define LEVEL_CMD_BLACKOUT                  (0x34)
#define LEVEL_CMD_GAMMA                     (0x35)
#define LEVEL_CMD_SET_BACKGROUND_MUSIC      (0x36)
#define LEVEL_CMD_SET_MENU_MUSIC            (0x37)
#define LEVEL_CMD_STOP_MUSIC                (0x38)
#define LEVEL_CMD_MACRO_OBJECTS             (0x39)
#define LEVEL_CMD_CMD3A                     (0x3A)
#define LEVEL_CMD_WHIRLPOOL                 (0x3B)
#define LEVEL_CMD_GET_OR_SET                (0x3C)
#define LEVEL_CMD_ADV_DEMO                  (0x3D)
#define LEVEL_CMD_CLEAR_DEMO_PTR            (0x3E)
#define LEVEL_CMD_JUMP_AREA                 (0x3F)
#endif

#define level_cmd_shift                     (sizeof(void *) >> 3)
#define level_cmd_offset(offset)            (((offset) & 3) | (((offset) & ~3) << level_cmd_shift))
#define level_cmd_get(cmd, type, offset)    (*(type *) (((u8 *) (cmd)) + level_cmd_offset(offset)))

typedef s32 (*LevelScriptPreprocessFunc)(u8, void *);
void level_script_preprocess(const LevelScript *script, LevelScriptPreprocessFunc func);

// Helpers defined by data/omm modules that were declared implicitly in the
// original OMM build.
u32 omm_mario_push_off_steep_floor(struct MarioState *m, u32 action, u32 actionArg);
s16 omm_find_mario_anim_flags_and_translation(struct Object *obj, s32 yaw, Vec3s translation);
bool vec3f_is_inside_box(Vec3f v, Vec3f boxMin, Vec3f boxMax);

// coopdx does not expose OMM's filesystem path macros.
#ifndef FS_SOUNDDIR
#define FS_SOUNDDIR "sound"
#endif

// Stub for coopdx's missing OMM gfx helper (implemented in omm_geo_shim.c).
Gfx *gfx_create_ortho_matrix(Gfx *dl);

#ifndef _SHIFTL
#define _SHIFTL(v, s, w)  ((u32) (((u32) (v) & (0xFFFF >> (16 - (w)))) << (s)))
#endif

#ifndef G_TEXTURE_GEN_INVERT
#define G_TEXTURE_GEN_INVERT                 0x000800
#endif

#ifndef gSPVertexExt
#define gSPVertexExt(pkt, v, n, v0) \
{ \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(G_VTXEXT, 24, 8) | _SHIFTL((n), 8, 16) | _SHIFTL((v0), 0, 8); \
    _g->words.w1 = (uintptr_t)(v); \
}
#endif
#ifndef gSPTrianglesExt
#define gSPTrianglesExt(pkt, tris, n) \
{ \
    Gfx *_g = (Gfx *)(pkt); \
    _g->words.w0 = _SHIFTL(G_TRIEXT, 24, 8) | _SHIFTL((n), 8, 16); \
    _g->words.w1 = (uintptr_t)(tris); \
}
#endif

#define OMM_NUM_LOADED_GRAPH_NODES 0x100

extern struct GraphNode *gLoadedGraphNodes[OMM_NUM_LOADED_GRAPH_NODES];
extern s8 gLoadedGraphNodeTypes[OMM_NUM_LOADED_GRAPH_NODES];
extern Vec3f gVec3fX;
extern Vec3f gVec3fZ;
extern u16 gRandomSeed;

enum AngleOrder { XYZ, YXZ, ZXY, ZYX, YZX, XZY };

// Minimal Cheater-compatible struct used by OMM cheat defines.
struct Cheats {
    bool EnableCheats;
    bool MoonJump;
    bool GodMode;
    bool Invincible;
    bool SuperSpeed;
    bool SuperResponsive;
    bool NoFallDamage;
    bool CapModifier;
    bool WalkOnHazards;
    bool NoDeathBarrier;
    bool BLJAnywhere;
    bool Responsive;
    bool ChaosMode;
    u32 ChaosControls;
    bool ChaosGreenDemon;
    s32 ChaosPlayAs;
    s32 PlayAs;
};
extern struct Cheats Cheats;
extern u8 gSpecialTripleJump;

// Stubs for the Cheater options menu (omm_options_menu.inl is not built).
extern bool optmenu_open;
void optmenu_toggle(void);
void func_80321080(s32 arg);
void main_pool_pop_state(void);
void launch_game(const char *path);

#ifndef OMM_GAME_CODE
#define OMM_GAME_CODE                               "sm64"
#endif

#ifndef INPUT_UNKNOWN_5
#define INPUT_UNKNOWN_5              INPUT_ZERO_MOVEMENT
#endif
#ifndef INTERACT_UNKNOWN_31
#define INTERACT_UNKNOWN_31          INTERACT_IGLOO_BARRIER
#endif

#define newcam_active                (gMarioState->area->camera->mode == CAMERA_MODE_NEWCAM)
#define configCameraMouse            configFreeCameraMouse
#define newcam_yaw                   gLakituState.yaw

#define configCameraAnalog           configFreeCameraAnalog
#define configCameraDegrade          configFreeCameraDegrade
#define configCameraXSens            configFreeCameraXSens
#define configCameraYSens            configFreeCameraYSens

#define vec3f_eq(a, b)              (memcmp(a, b, sizeof(Vec3f)) == 0)

// coopdx renames the intro entries compared to the original OMM port.
#undef level_script_splash_screen
#undef level_script_goddard_regular
#undef level_script_goddard_game_over
#define level_script_splash_screen                      level_intro_splash_screen
#define level_script_goddard_regular                    level_intro_mario_head_regular
#define level_script_goddard_game_over                  level_intro_mario_head_dizzy

#define WARP_OP_UNKNOWN_02           WARP_OP_SPIN_SHRINK
#define WARP_OP_UNKNOWN_01           WARP_OP_LOOK_UP

struct GraphNode *geo_layout_to_graph_node(struct DynamicPool *pool, const GeoLayout *geoLayout);
void *geo_get_geo_data(struct Object *o, s32 structSize, const u32 *displayListsOffsets, s32 numOffsets);
Gfx *geo_link_geo_data(s32 callContext, struct GraphNode *node, void *context);
Gfx *geo_link_geo_data_skip_mirror_obj(s32 callContext, struct GraphNode *node, void *context);
Gfx *gfx_copy_and_fill_null(Gfx *dest, const Gfx *src, s32 size, const Gfx *gfx);
Gfx *geo_move_from_camera(s32 callContext, struct GraphNode *node, void *context);
Gfx *geo_move_from_camera_mario_scale(s32 callContext, struct GraphNode *node, void *context);
Gfx *gfx_create_billboard(Gfx *gfx, Vtx *vtx, f32 left, f32 top, f32 width, f32 height,
                          s32 texX, s32 texY, s32 texW, s32 texH,
                          Vec3f billboardPos, Vec3f objPos, f32 cameraOffset, s16 roll,
                          u8 r, u8 g, u8 b, u8 a);

Vec3fp vec3f_get_nullspace(Vec3f dest1, Vec3f dest2, Vec3f v, Vec3f normal);
Vec3fp vec2f_to_3d_plane(Vec3f dest, Vec2f v, Vec3f planeBase, Vec3f planeX, Vec3f planeXScale, Vec3f planeY, Vec3f planeYScale);
Vec3fp vec3f_project_point(Vec3f dest, Vec3f p, Vec3f o, Vec3f n);
Vec3fp vec3f_project_vector(Vec3f dest, Vec3f v, Vec3f n);
Vec2fp vec2f_get_projected_point_on_line(Vec2f dest, f32 *t, Vec2f p, Vec2f a, Vec2f b);
void mtxf_transform(Mat4 dest, Vec3f translation, Vec3s rotation, Vec3f shear, Vec3f scale);
void mtxf_get_components(Mat4 m, Vec3f translation, Vec3s rotation, Vec3f shear, Vec3f scale);
void mtxf_get_rotation(Mat4 m, Vec3s rotation, enum AngleOrder order);

void geo_preprocess_object_graph_node(struct Object *o);
void geo_get_marios_anim_part_pos(struct GraphNode *node, Vec3f dest, s32 animPart);
void geo_get_marios_anim_part_mtx(struct GraphNode *node, Mat4 dest, s32 animPart);
f32  geo_get_marios_anim_part_height(struct GraphNode *node, s32 animPart);
f32  geo_get_marios_anim_part_distance(struct GraphNode *node, s32 animPart1, s32 animPart2);
bool geo_compute_capture_cappy_obj_transform(struct Object *o, s32 animParts, Mat4 transform);

void vtxn_interpolate(Vtx_tn *dest, Vtx_tn *from, Vtx_tn *to, f32 t);
extern bool gFrameInterpolation;

extern s8 gCourseNumToLevelNumTable[];

// Extra math helpers present in the OMM port but not in coopdx.
#define vec4f_copy(dest, src)        memcpy(dest, src, sizeof(Vec4f))
#define lerp_f(t, a, b)              ((a) + ((b) - (a)) * (t))
#define sqr_f(x)                     ((x) * (x))
void vec3f_to_2d_plane(Vec2f dest2d, f32 *dist2d, Vec3f src3d, Vec3f o, Vec3f n, Vec3f e1, Vec3f e2);
Vec3fp vec2f_and_dist_to_3d(Vec3f dest3d, Vec2f src2d, f32 dist2d, Vec3f o, Vec3f n, Vec3f e1, Vec3f e2);
Vec3fp vec3f_interpolate3(Vec3f dest, f32 t, Vec3f p0, f32 t0, Vec3f p1, f32 t1, Vec3f p2, f32 t2);
void vec3f_to_polar_coords(Vec3f v, f32 *dist, s16 *pitch, s16 *yaw);
void vtxv_interpolate(Vtx_t *dest, Vtx_t *from, Vtx_t *to, f32 t);
u16 srandom_u16(u16 seed);
f32 random_float_n1_p1(void);

// Frame interpolation data (OMM stats board / graphics helpers).
typedef struct {
    Gfx *pos;
    Gfx *gfx;
    Vtx *vtx;
    f32 x, x0, x1;
    f32 y, y0, y1;
    f32 z, z0, z1;
    f32 a, a0, a1;
    f32 s, s0, s1;
    f32 t, t0, t1;
    bool inited;
} InterpData;
void interp_data_update(InterpData *data, bool shouldInterp, Gfx *pos, Gfx *gfx, Vtx *vtx, f32 x, f32 y, f32 z, f32 a, f32 s, f32 t);
void interp_data_lerp(InterpData *data, f32 t);
void interp_data_reset(InterpData *data);

extern s32 gRedCoinsCollected;

bool find_first_hit_on_ray(Vec3f orig, Vec3f dir, Vec3f hitPos, f32 offset, f32 surfaceScale, u32 flags);

// FS helpers from the original OMM port (implemented in omm_surface_load_shim.c).
bool fs_seek(fs_file_t *file, s64 ofs);
s64 fs_fsize(const char *vpath);
const char *fs_cat_paths(sys_path_t dst, const char *path1, const char *path2);

// The OMM palette editor UI is excluded from the build; the palette editor is
// only queried by omm_mario_colors to apply a custom palette, which is never
// open during normal play.
s32 omm_palette_editor_get_current_palette(void);

#endif