#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#include "pc/dialog_table.h"
#undef OMM_ALL_HEADERS

#include "game/rendering_graph_node.h"
#include "data/omm/system/omm_geo_shim.h"

// OMM geo compatibility layer over the coopdx engine.
// Provides the geo callback helpers and math utilities that the
// OMM modules expect, implemented on top of the coopdx pipeline.

struct GraphNode *gLoadedGraphNodes[OMM_NUM_LOADED_GRAPH_NODES] = { 0 };
s8 gLoadedGraphNodeTypes[OMM_NUM_LOADED_GRAPH_NODES] = { 0 };
Vec3f gVec3fX = { 1.f, 0.f, 0.f };
Vec3f gVec3fZ = { 0.f, 0.f, 1.f };
u16 gRandomSeed = 0;
struct Cheats Cheats = { 0 };
u8 gSpecialTripleJump = 0;

// Removed in coopdx; legacy audio fade helper.
void func_80321080(s32 arg) {
    (void) arg;
}

// Options menu (Cheater mod) stubs: omm_options_menu.inl is not built.
bool optmenu_open = false;
void optmenu_toggle(void) { optmenu_open = !optmenu_open; }

// Removed in coopdx; clear_areas() already resets the level memory pool.
void main_pool_pop_state(void) { }

// Legacy launcher helper used by the "change game" option; no-op in coopdx.
void launch_game(const char *path) {
    (void) path;
}

static struct GraphNode *sGraphNodeCache[OMM_NUM_LOADED_GRAPH_NODES];
static const void *sGraphNodeCacheKeys[OMM_NUM_LOADED_GRAPH_NODES];
static u32 sGraphNodeCacheCount = 0;

struct GraphNode *geo_layout_to_graph_node(struct DynamicPool *pool, const GeoLayout *geoLayout) {
    if (!geoLayout) {
        return NULL;
    }

    // Cache check
    if (!pool) {
        for (u32 i = 0; i != sGraphNodeCacheCount; ++i) {
            if (sGraphNodeCacheKeys[i] == (const void *) geoLayout) {
                return sGraphNodeCache[i];
            }
        }
    }

    // Process the geo layout using the coopdx loader.
    extern struct GraphNode *process_geo_layout(struct DynamicPool *a0, void *segptr);
    struct GraphNode *graphNode = process_geo_layout(gGraphNodePool, (void *) geoLayout);

    // Cache the result
    if (!pool && graphNode) {
        if (sGraphNodeCacheCount < OMM_NUM_LOADED_GRAPH_NODES) {
            sGraphNodeCacheKeys[sGraphNodeCacheCount] = (const void *) geoLayout;
            sGraphNodeCache[sGraphNodeCacheCount] = graphNode;
            sGraphNodeCacheCount++;
        }
    }
    return graphNode;
}

void *geo_get_geo_data(struct Object *o, s32 structSize, const u32 *displayListsOffsets, s32 numOffsets) {
    if (!o->oGeoData) {
        void *geoData = omm_memory_new(gOmmMemoryPoolGeoData, structSize, o);
        Gfx **displayLists = (Gfx **) geoData;
        for (s32 i = 0; i != numOffsets; ++i) {
            displayLists[i] = (Gfx *) ((uintptr_t) geoData + (uintptr_t) displayListsOffsets[i]);
        }
        o->oGeoData = (void *) geoData;
    }
    return o->oGeoData;
}

Gfx *geo_link_geo_data(s32 callContext, struct GraphNode *node, UNUSED void *context) {
    if (gCurrGraphNodeObject && callContext == GEO_CONTEXT_RENDER) {
        struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
        struct GraphNodeDisplayList *displayListNode = (struct GraphNodeDisplayList *) node->next;
        Gfx **displayLists = (Gfx **) gCurrGraphNodeObject->oGeoData;
        if (displayLists) {
            displayListNode->displayList = (void *) displayLists[asGenerated->parameter];
        } else {
            displayListNode->displayList = NULL;
        }
    }
    return NULL;
}

Gfx *geo_link_geo_data_skip_mirror_obj(s32 callContext, struct GraphNode *node, void *context) {
    if (callContext == GEO_CONTEXT_RENDER) {
        if (gOmmGlobals->isMirrorObj) {
            struct GraphNodeDisplayList *displayListNode = (struct GraphNodeDisplayList *) node->next;
            displayListNode->displayList = NULL;
            return NULL;
        }
        return geo_link_geo_data(callContext, node, context);
    }
    return NULL;
}

Gfx *gfx_copy_and_fill_null(Gfx *dest, const Gfx *src, s32 size, const Gfx *gfx) {
    mem_cpy(dest, src, size);
    for (s32 i = 0, n = size / sizeof(Gfx); i != n; ++i) {
        Gfx *cmd = ((Gfx *) dest) + i;
        if (_SHIFTR(cmd->words.w0, 24, 8) == G_DL && cmd->words.w1 == (uintptr_t) NULL) {
            cmd->words.w1 = (uintptr_t) gfx;
            break;
        }
    }
    return dest;
}

static Gfx *__geo_move_from_camera(s32 callContext, struct GraphNode *node, UNUSED void *context, f32 offsetScale) {
    if (callContext == GEO_CONTEXT_RENDER && gCurGraphNodeObject) {
        f32 dist = vec3f_length(gCurGraphNodeObject->cameraToObject);
        if (dist != 0) {
            struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
            struct GraphNodeTranslation *translationNode = (struct GraphNodeTranslation *) node->next;
            f32 offset = offsetScale * (s16) asGenerated->parameter;
            translationNode->translation[0] = (s16) ((offset * gCurGraphNodeObject->cameraToObject[0]) / (dist * gCurGraphNodeObject->scale[0]));
            translationNode->translation[1] = (s16) ((offset * gCurGraphNodeObject->cameraToObject[1]) / (dist * gCurGraphNodeObject->scale[1]));
            translationNode->translation[2] = (s16) ((offset * gCurGraphNodeObject->cameraToObject[2]) / (dist * gCurGraphNodeObject->scale[2]));
        }
    }
    return NULL;
}

Gfx *geo_move_from_camera(s32 callContext, struct GraphNode *node, void *context) {
    return gCurGraphNodeObject ? __geo_move_from_camera(callContext, node, context, gCurGraphNodeObject->scale[1]) : NULL;
}

Gfx *geo_move_from_camera_mario_scale(s32 callContext, struct GraphNode *node, void *context) {
    return gMarioObject ? __geo_move_from_camera(callContext, node, context, gMarioObject->oScaleY) : NULL;
}

Gfx *gfx_create_billboard(
    Gfx *gfx, Vtx *vtx,
    f32 left, f32 top, f32 width, f32 height,
    s32 texX, s32 texY, s32 texW, s32 texH,
    Vec3f billboardPos, Vec3f objPos,
    f32 cameraOffset, s16 roll,
    u8 r, u8 g, u8 b, u8 a
) {
    // Billboard plane
    Vec3f camN, camE1, camE2;
    camN[0] = gCamera->pos[0] - billboardPos[0];
    camN[1] = gCamera->pos[1] - billboardPos[1];
    camN[2] = gCamera->pos[2] - billboardPos[2];
    vec3f_get_nullspace(camN, camE1, camE2, camN);
    vec3f_mul(camN, cameraOffset);

    // Vertices
    vtx[0] = (Vtx) { { { left,         top,          0 }, 0, { (texX       ) << 5, (texY       ) << 5 }, { r, g, b, a } } };
    vtx[1] = (Vtx) { { { left + width, top,          0 }, 0, { (texX + texW) << 5, (texY       ) << 5 }, { r, g, b, a } } };
    vtx[2] = (Vtx) { { { left + width, top + height, 0 }, 0, { (texX + texW) << 5, (texY + texH) << 5 }, { r, g, b, a } } };
    vtx[3] = (Vtx) { { { left,         top + height, 0 }, 0, { (texX       ) << 5, (texY + texH) << 5 }, { r, g, b, a } } };
    for (s32 i = 0; i != 4; ++i) {
        vec3f_rotate_around_n(vtx[i].v.ob, vtx[i].v.ob, gVec3fZ, roll);
        vec2f_to_3d_plane(vtx[i].v.ob, vtx[i].v.ob, billboardPos, camE1, gVec3fOne, camE2, gVec3fOne);
        vec3f_add(vtx[i].v.ob, camN);
        vec3f_sub(vtx[i].v.ob, objPos);
    }

    // Triangles
    gSPVertex(gfx++, vtx, 4, 0);
    gSP2Triangles(gfx++, 0, 1, 2, 0, 2, 3, 0, 0);
    return gfx;
}

//
// Math utilities
//

Vec3fp vec2f_to_3d_plane(Vec3f dest, Vec2f src, Vec3f o, Vec3f e1, Vec3f e1Scale, Vec3f e2, Vec3f e2Scale) {
    f32 x = src[0];
    f32 y = src[1];
    dest[0] = o[0] + x * e1[0] * e1Scale[0] + y * e2[0] * e2Scale[0];
    dest[1] = o[1] + x * e1[1] * e1Scale[1] + y * e2[1] * e2Scale[1];
    dest[2] = o[2] + x * e1[2] * e1Scale[2] + y * e2[2] * e2Scale[2];
    return dest;
}

Vec3fp vec3f_get_nullspace(Vec3f destAxisN, Vec3f destAxisE1, Vec3f destAxisE2, Vec3f n) {
    vec3f_set(destAxisN, n[0], n[1], n[2]);
    if (destAxisN[0] == 0.f && destAxisN[1] == 0.f) {
        vec3f_set(destAxisE1, 1.f, 0.f, 0.f);
        vec3f_set(destAxisE2, 0.f, 1.f, 0.f);
    } else {
        vec3f_set(destAxisE1, -destAxisN[1], destAxisN[0], 0.f);
        vec3f_set(destAxisE2, -destAxisN[0] * destAxisN[2], -destAxisN[1] * destAxisN[2], sqr_f(destAxisN[0]) + sqr_f(destAxisN[1]));
    }
    vec3f_normalize(destAxisN);
    vec3f_normalize(destAxisE1);
    vec3f_normalize(destAxisE2);
    return destAxisN;
}

Vec3fp vec3f_project_point(Vec3f dest, Vec3f p, Vec3f o, Vec3f n) {
    f32 dot = (p[0] - o[0]) * n[0] + (p[1] - o[1]) * n[1] + (p[2] - o[2]) * n[2];
    dest[0] = p[0] - dot * n[0];
    dest[1] = p[1] - dot * n[1];
    dest[2] = p[2] - dot * n[2];
    return dest;
}

Vec3fp vec3f_project_vector(Vec3f dest, Vec3f v, Vec3f n) {
    f32 dot = vec3f_dot(v, n);
    dest[0] = v[0] - dot * n[0];
    dest[1] = v[1] - dot * n[1];
    dest[2] = v[2] - dot * n[2];
    return dest;
}

void vec3f_to_polar_coords(Vec3f v, f32 *dist, s16 *pitch, s16 *yaw) {
    if (v[0] == 0.f && v[1] == 0.f && v[2] == 0.f) {
        if (dist) { *dist = 0.f; }
        if (pitch) { *pitch = 0; }
        if (yaw) { *yaw = 0; }
    } else {
        if (dist) { *dist = sqrtf(sqr_f(v[0]) + sqr_f(v[1]) + sqr_f(v[2])); }
        if (pitch) { *pitch = atan2s(sqrtf(sqr_f(v[0]) + sqr_f(v[2])), v[1]); }
        if (yaw) { *yaw = atan2s(v[2], v[0]); }
    }
}

void vec3f_to_2d_plane(Vec2f dest2d, f32 *dist2d, Vec3f src3d, Vec3f o, Vec3f n, Vec3f e1, Vec3f e2) {
    Vec3f op = { src3d[0] - o[0], src3d[1] - o[1], src3d[2] - o[2] };
    dest2d[0] = vec3f_dot(e1, op);
    dest2d[1] = vec3f_dot(e2, op);
    *dist2d = vec3f_dot(n, op);
}

Vec3fp vec3f_interpolate3(Vec3f dest, f32 t, Vec3f p0, f32 t0, Vec3f p1, f32 t1, Vec3f p2, f32 t2) {
    f32 tt0 = t  - t0;
    f32 tt1 = t  - t1;
    f32 tt2 = t  - t2;
    f32 t10 = t1 - t0;
    f32 t20 = t2 - t0;
    f32 t21 = t2 - t1;
    f32 m0 = (tt1 / -t10) * (tt2 / -t20);
    f32 m1 = (tt0 /  t10) * (tt2 / -t21);
    f32 m2 = (tt0 /  t20) * (tt1 /  t21);
    dest[0] = p0[0] * m0 + p1[0] * m1 + p2[0] * m2;
    dest[1] = p0[1] * m0 + p1[1] * m1 + p2[1] * m2;
    dest[2] = p0[2] * m0 + p1[2] * m1 + p2[2] * m2;
    return dest;
}

Vec2fp vec2f_get_projected_point_on_line(Vec2f dest, f32 *t, Vec2f p, Vec2f a, Vec2f b) {
    Vec2f ab = { b[0] - a[0], b[1] - a[1] };
    Vec2f ap = { p[0] - a[0], p[1] - a[1] };
    f32 dota = (ab[0] * ap[0]) + (ab[1] * ap[1]);
    f32 iab2 = 1.f / ((ab[0] * ab[0]) + (ab[1] * ab[1]));
    f32 abt = dota * iab2;
    dest[0] = a[0] + ab[0] * abt;
    dest[1] = a[1] + ab[1] * abt;
    if (t) *t = abt;
    return dest;
}

Vec3fp vec2f_and_dist_to_3d(Vec3f dest3d, Vec2f src2d, f32 dist2d, Vec3f o, Vec3f n, Vec3f e1, Vec3f e2) {
    f32 x = src2d[0];
    f32 y = src2d[1];
    dest3d[0] = o[0] + x * e1[0] + y * e2[0] + n[0] * dist2d;
    dest3d[1] = o[1] + x * e1[1] + y * e2[1] + n[1] * dist2d;
    dest3d[2] = o[2] + x * e1[2] + y * e2[2] + n[2] * dist2d;
    return dest3d;
}

u16 srandom_u16(u16 seed) {
    s32 r;
    if (seed == 0x9E37) {
        return 0;
    }
    r = (seed >> 1) + (((seed & 1) != 0) ? 0x4E35 : 0);
    return (u16) r;
}

f32 random_float_n1_p1(void) {
    gRandomSeed = srandom_u16(gRandomSeed);
    return ((f32) gRandomSeed / (f32) 0xFFFF) * 2.f - 1.f;
}

void vtxv_interpolate(Vtx_t *dest, Vtx_t *from, Vtx_t *to, f32 t) {
    dest->ob[0] =  lerp_f(t, from->ob[0], to->ob[0]);
    dest->ob[1] =  lerp_f(t, from->ob[1], to->ob[1]);
    dest->ob[2] =  lerp_f(t, from->ob[2], to->ob[2]);
    dest->tc[0] = (s16) lerp_f(t, from->tc[0], to->tc[0]);
    dest->tc[1] = (s16) lerp_f(t, from->tc[1], to->tc[1]);
    dest->cn[0] = (u8)  lerp_f(t, from->cn[0], to->cn[0]);
    dest->cn[1] = (u8)  lerp_f(t, from->cn[1], to->cn[1]);
    dest->cn[2] = (u8)  lerp_f(t, from->cn[2], to->cn[2]);
    dest->cn[3] = (u8)  lerp_f(t, from->cn[3], to->cn[3]);
}

void vtxn_interpolate(Vtx_tn *dest, Vtx_tn *from, Vtx_tn *to, f32 t) {
    dest->ob[0] = lerp_f(t, from->ob[0], to->ob[0]);
    dest->ob[1] = lerp_f(t, from->ob[1], to->ob[1]);
    dest->ob[2] = lerp_f(t, from->ob[2], to->ob[2]);
    dest->tc[0] = (s16) lerp_f(t, from->tc[0], to->tc[0]);
    dest->tc[1] = (s16) lerp_f(t, from->tc[1], to->tc[1]);
    dest->n[0]  = (s8)  lerp_f(t, from->n[0], to->n[0]);
    dest->n[1]  = (s8)  lerp_f(t, from->n[1], to->n[1]);
    dest->n[2]  = (s8)  lerp_f(t, from->n[2], to->n[2]);
    dest->a     = (u8)  lerp_f(t, from->a, to->a);
}

#define IS_ZERO(x)      ((x) > -0.0001f && (x) < +0.0001f)
#define MTXF_33_1(m)    m[3][3] = 1.f
#define vec3f_ortho(v1, v2, shear) { v1[0] -= v2[0] * (shear); v1[1] -= v2[1] * (shear); v1[2] -= v2[2] * (shear); }
#define vec3f_unscale(v, scale, default) if (IS_ZERO(scale)) { vec3f_copy(v, default); } else { vec3f_mul(v, 1.f / (scale)); }
#define shear_unscale(shear, scale) if (IS_ZERO(scale)) { shear = 0; } else { shear /= (scale); }

#define ANGLES(o, a0, a0x, a0m, a1, a1x, a1mY, a1mX, a2, a2x, a2mY, a2mX, a1e, a1ex, a1emY, a1emX, a2e) \
if (order == o) { \
    f32 ax, ay, az; \
    a0 = asinf(a0x * clamp_f(a0m, -1, 1)); \
    if (abs_f(a0m) < 0.999999f) { \
        a1 = atan2f(a1x * a1mY, a1mX); \
        a2 = atan2f(a2x * a2mY, a2mX); \
    } else { \
        a1e = atan2f(a1ex * a1emY, a1emX); \
        a2e = 0; \
    } \
    vec3s_set(rotation, ax * (0x8000 / M_PI), ay * (0x8000 / M_PI), az * (0x8000 / M_PI)); \
    return; \
}

void mtxf_get_rotation(Mat4 m, Vec3s rotation, enum AngleOrder order) {
    f32 m00 = m[0][0]; f32 m10 = m[1][0]; f32 m20 = m[2][0];
    f32 m01 = m[0][1]; f32 m11 = m[1][1]; f32 m21 = m[2][1];
    f32 m02 = m[0][2]; f32 m12 = m[1][2]; f32 m22 = m[2][2];
    ANGLES(XYZ, ay, +1, m20, ax, -1, m21, m22, az, -1, m10, m00, ax, +1, m12, m11, az);
    ANGLES(YXZ, ax, -1, m21, ay, +1, m20, m22, az, +1, m01, m11, ay, -1, m02, m00, az);
    ANGLES(ZXY, ax, +1, m12, ay, -1, m02, m22, az, -1, m10, m11, az, +1, m01, m00, ay);
    ANGLES(ZYX, ay, -1, m02, ax, +1, m12, m22, az, +1, m01, m00, az, -1, m10, m11, ax);
    ANGLES(YZX, az, +1, m01, ax, -1, m21, m11, ay, -1, m02, m00, ay, +1, m20, m22, ax);
    ANGLES(XZY, az, -1, m10, ax, +1, m12, m11, ay, +1, m20, m00, ax, -1, m21, m22, ay);
}

void mtxf_get_components(Mat4 m, Vec3f translation, Vec3s rotation, Vec3f shear, Vec3f scale) {
    // Translation
    vec3f_copy(translation, m[3]);

    // Axes
    Vec3f xAxis, yAxis, zAxis;
    vec3f_copy(xAxis, m[0]);
    vec3f_copy(yAxis, m[1]);
    vec3f_copy(zAxis, m[2]);

    // X scale
    scale[0] = vec3f_length(xAxis);
    vec3f_unscale(xAxis, scale[0], gVec3fX);

    // XY shear
    shear[0] = vec3f_dot(xAxis, yAxis);
    vec3f_ortho(yAxis, xAxis, shear[0]);

    // Y scale
    scale[1] = vec3f_length(yAxis);
    vec3f_unscale(yAxis, scale[1], gVec3fY);
    shear_unscale(shear[0], scale[1]);

    // XZ shear
    shear[1] = vec3f_dot(xAxis, zAxis);
    vec3f_ortho(zAxis, xAxis, shear[1]);

    // YZ shear
    shear[2] = vec3f_dot(yAxis, zAxis);
    vec3f_ortho(zAxis, yAxis, shear[2]);

    // Z scale
    scale[2] = vec3f_length(zAxis);
    vec3f_unscale(zAxis, scale[2], gVec3fZ);
    shear_unscale(shear[1], scale[2]);
    shear_unscale(shear[2], scale[2]);

    // (xAxis, yAxis, zAxis) is now an orthonormal coordinate system.
    // Check for a system flip, and if that's the case, negate axes and scaling.
    Vec3f yzCross;
    if (vec3f_dot(xAxis, vec3f_cross(yzCross, yAxis, zAxis)) < 0.f) {
        vec3f_mul(scale, -1);
        vec3f_mul(xAxis, -1);
        vec3f_mul(yAxis, -1);
        vec3f_mul(zAxis, -1);
    }

    // Rotation
    Mat4 mRot;
    mtxf_identity(mRot);
    vec3f_copy(mRot[0], xAxis);
    vec3f_copy(mRot[1], yAxis);
    vec3f_copy(mRot[2], zAxis);
    mtxf_get_rotation(mRot, rotation, YXZ);
}

void mtxf_transform(Mat4 dest, Vec3f translation, Vec3s rotation, Vec3f shear, Vec3f scale) {
    mtxf_zero(dest);
    MTXF_33_1(dest);

    // Scale
    dest[0][0] = scale[0];
    dest[1][1] = scale[1];
    dest[2][2] = scale[2];

    // Shear
    dest[1][0] = dest[1][1] * shear[0];
    dest[2][0] = dest[2][2] * shear[1];
    dest[2][1] = dest[2][2] * shear[2];

    // Rotate
    Mat4 mRot;
    mtxf_rotate_zxy_and_translate(mRot, gVec3fZero, rotation);
    mtxf_mul(dest, dest, mRot);

    // Translate
    vec3f_copy(dest[3], translation);
}

//
// Mario anim part helpers.
// coopdx fills gCurMarioBodyState->animPartsPos/Rot during rendering, so we
// read those directly instead of replicating OMM's geo preprocessing pipeline.
//

void geo_preprocess_object_graph_node(struct Object *o) {
    // No-op: coopdx computes anim parts during the regular render pass.
}

static struct MarioBodyState *__shim_mario_body_state(void) {
    extern struct MarioBodyState *gCurMarioBodyState;
    return gCurMarioBodyState;
}

void geo_get_marios_anim_part_pos(struct GraphNode *node, Vec3f dest, s32 animPart) {
    struct MarioBodyState *state = __shim_mario_body_state();
    if (state != NULL && animPart > MARIO_ANIM_PART_NONE && animPart < MARIO_ANIM_PART_MAX) {
        vec3f_copy(dest, state->animPartsPos[animPart]);
    } else {
        vec3f_zero(dest);
    }
}

void geo_get_marios_anim_part_mtx(struct GraphNode *node, Mat4 dest, s32 animPart) {
    struct MarioBodyState *state = __shim_mario_body_state();
    mtxf_identity(dest);
    if (state != NULL && animPart > MARIO_ANIM_PART_NONE && animPart < MARIO_ANIM_PART_MAX) {
        mtxf_rotate_zxy_and_translate(dest, state->animPartsPos[animPart], state->animPartsRot[animPart]);
    }
}

f32 geo_get_marios_anim_part_height(struct GraphNode *node, s32 animPart) {
    Vec3f pos;
    geo_get_marios_anim_part_pos(node, pos, animPart);
    return pos[1];
}

f32 geo_get_marios_anim_part_distance(struct GraphNode *node, s32 animPart1, s32 animPart2) {
    Vec3f pos1, pos2;
    geo_get_marios_anim_part_pos(node, pos1, animPart1);
    geo_get_marios_anim_part_pos(node, pos2, animPart2);
    return vec3f_dist(pos1, pos2);
}

bool geo_compute_capture_cappy_obj_transform(struct Object *o, s32 animParts, Mat4 transform) {
    // Fallback: place the cap at the object's current gfx transform (world space).
    if (o != NULL && o->oGraphNode != NULL) {
        mtxf_rotate_zxy_and_translate(transform, o->oGfxPos, o->oGfxAngle);
        mtxf_scale_vec3f(transform, transform, o->oGfxScale);
        return true;
    }
    return false;
}

//
// Extra math helpers present in the OMM port but not in coopdx.
//

void interp_data_update(InterpData *data, bool shouldInterp, Gfx *pos, Gfx *gfx, Vtx *vtx, f32 x, f32 y, f32 z, f32 a, f32 s, f32 t) {
    if (gFrameInterpolation && shouldInterp && data->inited) {
        data->pos = pos;
        data->x0 = data->x1;
        data->y0 = data->y1;
        data->z0 = data->z1;
        data->a0 = data->a1;
        data->s0 = data->s1;
        data->t0 = data->t1;
    } else {
        data->pos = NULL;
        data->x0 = x;
        data->y0 = y;
        data->z0 = z;
        data->a0 = a;
        data->s0 = s;
        data->t0 = t;
    }
    data->gfx = gfx;
    data->vtx = vtx;
    data->x1 = x;
    data->y1 = y;
    data->z1 = z;
    data->a1 = a;
    data->s1 = s;
    data->t1 = t;
    data->inited = true;
}

void interp_data_lerp(InterpData *data, f32 t) {
    data->x = lerp_f(t, data->x0, data->x1);
    data->y = lerp_f(t, data->y0, data->y1);
    data->z = lerp_f(t, data->z0, data->z1);
    data->a = lerp_f(t, data->a0, data->a1);
    data->s = lerp_f(t, data->s0, data->s1);
    data->t = lerp_f(t, data->t0, data->t1);
}

void interp_data_reset(InterpData *data) {
    memset(data, 0, sizeof(InterpData));
}

s32 gRedCoinsCollected = 0;

// Walks a level script, calling func(type, cmd) for each command, mirroring the
// OMM preprocess helper. coopdx executes level scripts through its own engine,
// so this only needs to support the commands omm_level.c inspects.
struct LevelCommand {
    u8 type;
    u8 size;
};
bool vec3f_is_inside_box(Vec3f v, Vec3f boxMin, Vec3f boxMax) {
    return boxMin[0] < v[0] && v[0] < boxMax[0] &&
           boxMin[1] < v[1] && v[1] < boxMax[1] &&
           boxMin[2] < v[2] && v[2] < boxMax[2];
}

// Stubs for modules excluded from the coopdx build (OMM custom UI renderer,
// dialog system, palette editor, etc.). The normal game never calls these in a
// way that requires real behavior.
Gfx *gfx_create_ortho_matrix(Gfx *dl) { return dl; }
void gfx_clear_frame_dialog(void) {}
void gfx_clear_frame_hud(void) {}
void omm_render_course_complete_init(void) {}
void omm_loading_screen_start(void) {}
bool omm_palette_editor_is_opening(void) { return false; }
bool omm_palette_editor_is_open(void) { return false; }
bool omm_palette_editor_is_closing(void) { return false; }
void omm_palette_editor_set_open(void) {}
void omm_palette_editor_set_closed(void) {}

static struct DialogEntry sOmmDialogFallback = { 0 };
struct DialogEntry *omm_dialog_get_entry(void **dialogTable, s16 dialogId) {
    if (dialogId >= OMM_DIALOG_START_INDEX) {
        return &sOmmDialogFallback;
    }
    if (dialogTable) {
        return ((struct DialogEntry **) dialogTable)[dialogId];
    }
    return dialog_table_get(dialogId);
}

void level_script_preprocess(const LevelScript *script, LevelScriptPreprocessFunc func) {
    struct LevelCommand *cmd = (struct LevelCommand *) script;
    struct LevelCommand *stack[32];
    s32 stackTop = 0;
    u64 count = 0;

    while (cmd != NULL) {
#ifdef TARGET_ANDROID
        if ((++count & 0xFFFFF) == 0) {
            extern int __android_log_print(int prio, const char *tag, const char *fmt, ...);
            __android_log_print(6, "OMMINIT", "preprocess iter=%llu cmd=%p type=%u func=%p", (unsigned long long) count, (void *) cmd, (unsigned) cmd->type, (void *) func);
        }
        if (count > 20000000) {
            extern int __android_log_print(int prio, const char *tag, const char *fmt, ...);
            __android_log_print(6, "OMMINIT", "preprocess LOOP CAP cmd=%p type=%u func=%p", (void *) cmd, (unsigned) cmd->type, (void *) func);
            return;
        }
#endif
        u8 type = cmd->type;
        s32 action = func(type, (void *) cmd);
        switch (action) {
            case LEVEL_SCRIPT_STOP:
                return;
            case LEVEL_SCRIPT_RETURN:
                cmd = (stackTop > 0) ? stack[--stackTop] : NULL;
                break;
            default: {
                struct LevelCommand *next = (struct LevelCommand *) ((u8 *) cmd + (cmd->size << CMD_SIZE_SHIFT));
                switch (type) {
                    case LEVEL_CMD_JUMP_LINK:
                    case LEVEL_CMD_JUMP_LINK_PUSH_ARG:
                        if (stackTop < 32) { stack[stackTop++] = next; }
                        cmd = (struct LevelCommand *) segmented_to_virtual(level_cmd_get(cmd, void *, 12));
                        break;
                    case LEVEL_CMD_EXECUTE:
                        if (stackTop < 32) { stack[stackTop++] = next; }
                        cmd = (struct LevelCommand *) segmented_to_virtual(level_cmd_get(cmd, void *, 12));
                        break;
                    default:
                        cmd = next;
                        break;
                }
            } break;
        }
    }
}

// The OMM palette editor is excluded from the build; no custom palette is
// ever active, so the default presets are always used.
s32 omm_palette_editor_get_current_palette(void) {
    return -1;
}
