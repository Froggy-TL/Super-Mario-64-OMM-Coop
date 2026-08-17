#include "data/omm/omm_includes.h"
#include "game/object_list_processor.h"
#include "game/object_helpers.h"
#include "object_constants.h"
#include "surface_load.h"
#include "surface_terrains.h"
#include "data/omm/engine/headers/include/surface_data.h"

s32 ray_surface_intersect(Vec3f orig, Vec3f dir, f32 dir_length, struct Surface *surface, Vec3f hit_pos, f32 *length);

void obj_update(struct Object *o) {
    if (o == NULL) { return; }
    struct Object *prev = gCurrentObject;
    gCurrentObject = o;
    cur_obj_update();
    gCurrentObject = prev;
}

static s32 omm_get_surface_index(struct Object *obj, struct Surface *surf) {
    s32 index = 0;
    for (;;) {
        struct Surface *s = obj_get_surface_from_index(obj, index);
        if (s == NULL) { return -1; }
        if (s == surf) { return index; }
        index++;
    }
}

void clear_surface_data(struct SurfaceData *data) {
    memset(data, 0, sizeof(struct SurfaceData));
}

void get_surface_data(struct SurfaceData *data, struct Surface *surf) {
    if (surf->object) {
        data->dynamic = true;
        data->object = surf->object;
        data->behavior = surf->object->behavior;
        data->collision = surf->object->collisionData;
        data->index = omm_get_surface_index(surf->object, surf);
        vec3f_copy(data->pos, &surf->object->oPosX);
        vec3s_copy(data->angle, &surf->object->oFaceAngleYaw);
        vec3f_copy(data->scale, &surf->object->header.gfx.scale[0]);
    } else {
        data->dynamic = false;
        data->surface = surf;
    }
}

struct Surface *get_surface_from_data(const struct SurfaceData *data) {
    if (data->dynamic) {
        if (data->object->activeFlags == ACTIVE_FLAG_DEACTIVATED) { return NULL; }
        if (data->object->behavior != data->behavior) { return NULL; }
        if (data->object->collisionData != data->collision) { return NULL; }
        return obj_get_surface_from_index(data->object, data->index);
    }
    return data->surface;
}

struct Surface *get_next_surface(struct Surface *surf) {
    if (surf && surf->object) {
        s32 index = omm_get_surface_index(surf->object, surf);
        if (index >= 0) {
            return obj_get_surface_from_index(surf->object, index + 1);
        }
    }
    return NULL;
}

void omm_load_object_collision_model(void) {
    load_object_collision_model();
}

void obj_load_collision_model(struct Object *o) {
    if (o == NULL) { return; }
    struct Object *prev = gCurrentObject;
    gCurrentObject = o;
    load_object_collision_model();
    o->oCollisionDistance = LEVEL_BOUNDARY_MAX;
    o->oDistanceToMario = dist_between_objects(o, gMarioObject);
    gCurrentObject = prev;
}

void recompute_surface_parameters(struct Surface *surf) {
    if (surf == NULL) { return; }
    f32 x1 = surf->vertex1[0], y1 = surf->vertex1[1], z1 = surf->vertex1[2];
    f32 x2 = surf->vertex2[0], y2 = surf->vertex2[1], z2 = surf->vertex2[2];
    f32 x3 = surf->vertex3[0], y3 = surf->vertex3[1], z3 = surf->vertex3[2];
    f32 nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
    f32 ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
    f32 nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
    f32 mag = sqrtf(nx * nx + ny * ny + nz * nz);
    if (mag < 0.0001f) { return; }
    mag = 1.f / mag;
    surf->normal.x = nx * mag;
    surf->normal.y = ny * mag;
    surf->normal.z = nz * mag;
    surf->originOffset = -(surf->normal.x * x1 + surf->normal.y * y1 + surf->normal.z * z1);
    surf->lowerY = min_3_s(surf->vertex1[1], surf->vertex2[1], surf->vertex3[1]) - 5;
    surf->upperY = max_3_s(surf->vertex1[1], surf->vertex2[1], surf->vertex3[1]) + 5;
}

void omm_surface_add_all_surfaces_again(void) {
}

void omm_debug_surfaces_print_info(s32 x, s32 y) {
    (void) x;
    (void) y;
}

static bool omm_ray_hit_is_checked(RayCollisionData *hits, struct Surface *surf) {
    for (s32 i = 0; i < hits->count; i++) {
        if (hits->hits[i].surf == surf) { return true; }
    }
    return false;
}

static void find_ray_hits_from_surface_list(struct SurfaceNode *list, Vec3f orig, Vec3f ndir, f32 maxDist, f32 surfaceScale, RayCollisionData *hits, bool rejectNoCamCol) {
    f32 upperY = max_f(orig[1], orig[1] + ndir[1] * maxDist);
    f32 lowerY = min_f(orig[1], orig[1] + ndir[1] * maxDist);
    for (; list != NULL && hits->count < MAX_RAYCAST_COL_HITS; list = list->next) {
        struct Surface *surf = list->surface;

        // Reject surface out of vertical bounds
        if (surf->lowerY > upperY || surf->upperY < lowerY) { continue; }

        // Reject no-cam collision and vanish cap walls surfaces
        if (rejectNoCamCol && (
            (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) ||
            (surf->type == SURFACE_NO_CAM_COLLISION) ||
            (surf->type == SURFACE_NO_CAM_COLLISION_77) ||
            (surf->type == SURFACE_NO_CAM_COL_VERY_SLIPPERY) ||
            (surf->type == SURFACE_NO_CAM_COL_SLIPPERY) ||
            (surf->type == SURFACE_SWITCH) ||
            (surf->type == SURFACE_VANISH_CAP_WALLS))) {
            continue;
        }

        // Reject camera boundary surfaces
        if (!rejectNoCamCol && surf->type == SURFACE_CAMERA_BOUNDARY) { continue; }

        // Exclude already checked surfaces
        if (omm_ray_hit_is_checked(hits, surf)) { continue; }

        // Check intersection
        Vec3f v0; vec3s_to_vec3f(v0, surf->vertex1);
        Vec3f v1; vec3s_to_vec3f(v1, surf->vertex2);
        Vec3f v2; vec3s_to_vec3f(v2, surf->vertex3);
        if (surfaceScale != 1.f) {
            Vec3f vc = {
                (v0[0] + v1[0] + v2[0]) / 3.f,
                (v0[1] + v1[1] + v2[1]) / 3.f,
                (v0[2] + v1[2] + v2[2]) / 3.f,
            };
            Vec3f v0c; vec3f_mul(vec3f_dif(v0c, v0, vc), surfaceScale);
            Vec3f v1c; vec3f_mul(vec3f_dif(v1c, v1, vc), surfaceScale);
            Vec3f v2c; vec3f_mul(vec3f_dif(v2c, v2, vc), surfaceScale);
            vec3f_sum(v0, vc, v0c);
            vec3f_sum(v1, vc, v1c);
            vec3f_sum(v2, vc, v2c);
        }
        Vec3f e1; vec3f_dif(e1, v1, v0);
        Vec3f e2; vec3f_dif(e2, v2, v0);
        Vec3f vh; vec3f_cross(vh, ndir, e2);

        // Perpendicular?
        f32 dot = vec3f_dot(e1, vh);
        if (dot > -0.01f && dot < 0.01f) { continue; }
        f32 invdot = 1.f / dot;

        // Contact?
        Vec3f vo; vec3f_dif(vo, orig, v0);
        f32 u = invdot * vec3f_dot(vo, vh);
        if (u < 0.f || u > 1.f) { continue; }
        Vec3f voe1; vec3f_cross(voe1, vo, e1);
        f32 v = invdot * vec3f_dot(ndir, voe1);
        if (v < 0.f || (u + v) > 1.f) { continue; }

        // Compute dist
        f32 dist = invdot * vec3f_dot(e2, voe1);
        if (dist < 0.01f || dist > maxDist) { continue; }

        // Successful hit
        RayHit hit;
        vec3f_copy(hit.pos, ndir);
        vec3f_mul(hit.pos, dist);
        vec3f_add(hit.pos, orig);
        hit.dist = dist;
        hit.surf = surf;
        hit.ratio = dist / maxDist;

        // Reject hit if not opposed to the ray direction
        if (vec3f_dot(ndir, &surf->normal.x) > 0.f) { continue; }

        // Add hit to the hits sorted list
        for (s32 i = 0; i <= hits->count; ++i) {
            if (i == hits->count || hits->hits[i].dist > hit.dist) {
                memmove(hits->hits + i + 1, hits->hits + i, sizeof(RayHit) * (hits->count - i));
                hits->hits[i] = hit;
                hits->count++;
                break;
            }
        }
    }
}

static void find_ray_hits_on_cell(s16 cx, s16 cz, Vec3f orig, Vec3f ndir, f32 maxDist, f32 surfaceScale, RayCollisionData *hits, u32 flags) {
    if (cx >= 0 && cx < NUM_CELLS && cz >= 0 && cz < NUM_CELLS) {

        // Walls
        if (flags & RAYCAST_FLAG_WALLS) {
            find_ray_hits_from_surface_list(gStaticSurfacePartition[cz][cx][SPATIAL_PARTITION_WALLS].next, orig, ndir, maxDist, surfaceScale, hits, flags & RAYCAST_FLAG_NO_CAM_COL);
            find_ray_hits_from_surface_list(gDynamicSurfacePartition[cz][cx][SPATIAL_PARTITION_WALLS].next, orig, ndir, maxDist, surfaceScale, hits, flags & RAYCAST_FLAG_NO_CAM_COL);
        }

        // Floors
        if ((flags & RAYCAST_FLAG_FLOORS) && ndir[1] < +0.99f) {
            find_ray_hits_from_surface_list(gStaticSurfacePartition[cz][cx][SPATIAL_PARTITION_FLOORS].next, orig, ndir, maxDist, surfaceScale, hits, flags & RAYCAST_FLAG_NO_CAM_COL);
            find_ray_hits_from_surface_list(gDynamicSurfacePartition[cz][cx][SPATIAL_PARTITION_FLOORS].next, orig, ndir, maxDist, surfaceScale, hits, flags & RAYCAST_FLAG_NO_CAM_COL);
        }

        // Ceilings
        if ((flags & RAYCAST_FLAG_CEILS) && ndir[1] > -0.99f) {
            find_ray_hits_from_surface_list(gStaticSurfacePartition[cz][cx][SPATIAL_PARTITION_CEILS].next, orig, ndir, maxDist, surfaceScale, hits, flags & RAYCAST_FLAG_NO_CAM_COL);
            find_ray_hits_from_surface_list(gDynamicSurfacePartition[cz][cx][SPATIAL_PARTITION_CEILS].next, orig, ndir, maxDist, surfaceScale, hits, flags & RAYCAST_FLAG_NO_CAM_COL);
        }
    }
}

s32 find_collisions_on_ray(Vec3f orig, Vec3f dir, RayCollisionData *hits, f32 surfaceScale, u32 flags) {
    hits->count = 0;
    f32 maxDist = vec3f_length(dir);
    if (maxDist < 0.0001f) { return 0; }

    // Normalized dir
    Vec3f ndir;
    vec3f_copy(ndir, dir);
    vec3f_mul(ndir, 1.f / maxDist);

    // Steps and cells
    f32 steps = 4.f * max_f(abs_f(dir[0]), abs_f(dir[2])) / CELL_SIZE;
    f32 dx = dir[0] / (steps * CELL_SIZE);
    f32 dz = dir[2] / (steps * CELL_SIZE);
    f32 cx = (orig[0] + LEVEL_BOUNDARY_MAX) / CELL_SIZE;
    f32 cz = (orig[2] + LEVEL_BOUNDARY_MAX) / CELL_SIZE;

    // DDA
    for (s32 i = 0; i <= (s32) steps; ++i, cx += dx, cz += dz) {
        find_ray_hits_on_cell((s16) cx, (s16) cz, orig, ndir, maxDist, surfaceScale, hits, flags);
    }
    return hits->count;
}

static bool disk_overlaps_point(Vec2f point, f32 radius2) {
    return vec2f_dot(point, point) <= radius2;
}

static bool disk_overlaps_segment(Vec2f point0, Vec2f point1, f32 radius2) {
    Vec2f direction; vec2f_dif(direction, point0, point1);
    f32 dot = vec2f_dot(point0, direction);
    if (dot <= 0) {
        return vec2f_dot(point0, point0) <= radius2;
    }
    f32 length2 = vec2f_dot(direction, direction);
    if (dot >= length2) {
        return vec2f_dot(point1, point1) <= radius2;
    }
    f32 dotperp = direction[0] * point0[1] - direction[1] * point0[0];
    return dotperp * dotperp <= length2 * radius2;
}

static bool disk_overlaps_polygon(Vec2f *points, u32 count, f32 radius2) {
    bool positive = false, negative = false;
    for (u32 i0 = count - 1, i1 = 0; i1 != count && !(positive && negative); i0 = i1++) {
        Vec2f direction; vec2f_dif(direction, points[i0], points[i1]);
        f32 dotperp = points[i0][0] * direction[1] - points[i0][1] * direction[0];
        positive |= dotperp > 0;
        negative |= dotperp < 0;
    }
    if (!positive || !negative) {
        return true;
    }
    for (u32 i0 = count - 1, i1 = 0; i1 != count; i0 = i1++) {
        if (disk_overlaps_segment(points[i0], points[i1], radius2)) {
            return true;
        }
    }
    return false;
}

static u32 get_index_of_lowest_vertex(Vec3s *vertices) {
    s16 ymin = min_3_s(vertices[0][1], vertices[1][1], vertices[2][1]);
    if (ymin == vertices[0][1]) return 0;
    if (ymin == vertices[1][1]) return 1;
    return 2;
}

static u32 get_index_of_highest_vertex(Vec3s *vertices) {
    s16 ymin = max_3_s(vertices[0][1], vertices[1][1], vertices[2][1]);
    if (ymin == vertices[2][1]) return 2;
    if (ymin == vertices[1][1]) return 1;
    return 0;
}

static void compute_dir(Vec2f dest, Vec2f point0, Vec2f point1, f32 denom) {
    dest[0] = (point0[0] - point1[0]) / denom;
    dest[1] = (point0[1] - point1[1]) / denom;
}

static void compute_polygon_point(Vec2f dest, Vec2f point, f32 mult, Vec2f dir) {
    dest[0] = point[0] + mult * dir[0];
    dest[1] = point[1] + mult * dir[1];
}

// Credits to https://www.geometrictools.com/GTE/Mathematics/IntrTriangle3Cylinder3.h
bool surface_intersects_cylinder(struct Surface *surf, Vec3f pos, f32 radius, f32 height, f32 downOffset) {

    // Sort the triangle vertices so that y[0] <= y[1] <= y[2]
    Vec3s vertices[3];
    vec3s_copy(vertices[0], surf->vertex1);
    vec3s_copy(vertices[1], surf->vertex2);
    vec3s_copy(vertices[2], surf->vertex3);
    u32 li = get_index_of_lowest_vertex(vertices);
    u32 hi = get_index_of_highest_vertex(vertices);
    u32 mi = 3 - (li + hi);

    // Compute the coordinates of the surface vertices with the center of the cylinder as the origin
    f32 heightHalf = height / 2;
    Vec3f xyz[3] = {
        { vertices[li][0] - pos[0], vertices[li][1] - (pos[1] + heightHalf - downOffset), vertices[li][2] - pos[2] },
        { vertices[mi][0] - pos[0], vertices[mi][1] - (pos[1] + heightHalf - downOffset), vertices[mi][2] - pos[2] },
        { vertices[hi][0] - pos[0], vertices[hi][1] - (pos[1] + heightHalf - downOffset), vertices[hi][2] - pos[2] },
    };

    // y components
    f32 y[3] = { xyz[0][1], xyz[1][1], xyz[2][1] };

    // Early exit 1: triangle is strictly below/above cylinder
    if (y[2] < -heightHalf || y[0] > +heightHalf) {
        return false;
    }

    // xz components
    Vec2f xz[3] = {
        { xyz[0][0], xyz[0][2] },
        { xyz[1][0], xyz[1][2] },
        { xyz[2][0], xyz[2][2] },
    };

    // Early exit 2: triangle is not in the cylinder horizontal range
    f32 radius2 = radius * radius;
    if (!disk_overlaps_polygon(xz, 3, radius2)) {
        return false;
    }

    // Early exit 3: if the triangle is vertically inside the cylinder,
    // there is intersection, thanks to the previous check
    if (-heightHalf <= y[0] && y[2] <= heightHalf) {
        return true;
    }

    // Lowest is below the cylinder
    if (y[0] < -heightHalf) {

        // Highest is above the cylinder
        if (y[2] > heightHalf) {

            // Middle is at the top or above the cylinder
            if (y[1] >= heightHalf) {
                f32 numerNeg0 = -heightHalf - y[0];
                f32 numerPos0 = +heightHalf - y[0];
                f32 denom10 = y[1] - y[0];
                f32 denom20 = y[2] - y[0];
                Vec2f dir20; compute_dir(dir20, xz[2], xz[0], denom20);
                Vec2f dir10; compute_dir(dir10, xz[1], xz[0], denom10);
                Vec2f polygon[4];
                compute_polygon_point(polygon[0], xz[0], numerNeg0, dir20);
                compute_polygon_point(polygon[1], xz[0], numerNeg0, dir10);
                compute_polygon_point(polygon[2], xz[0], numerPos0, dir10);
                compute_polygon_point(polygon[3], xz[0], numerPos0, dir20);
                return disk_overlaps_polygon(polygon, 4, radius2);
            }

            // Middle is at the bottom or below the cylinder
            if (y[1] <= -heightHalf) {
                f32 numerNeg2 = -heightHalf - y[2];
                f32 numerPos2 = +heightHalf - y[2];
                f32 denom02 = y[0] - y[2];
                f32 denom12 = y[1] - y[2];
                Vec2f dir02; compute_dir(dir02, xz[0], xz[2], denom02);
                Vec2f dir12; compute_dir(dir12, xz[1], xz[2], denom12);
                Vec2f polygon[4];
                compute_polygon_point(polygon[0], xz[2], numerNeg2, dir02);
                compute_polygon_point(polygon[1], xz[2], numerNeg2, dir12);
                compute_polygon_point(polygon[2], xz[2], numerPos2, dir12);
                compute_polygon_point(polygon[3], xz[2], numerPos2, dir02);
                return disk_overlaps_polygon(polygon, 4, radius2);
            }

            // Middle is inside the cylinder
            f32 numerNeg0 = -heightHalf - y[0];
            f32 numerPos0 = +heightHalf - y[0];
            f32 numerNeg1 = -heightHalf - y[1];
            f32 numerPos1 = +heightHalf - y[1];
            f32 denom20 = y[2] - y[0];
            f32 denom01 = y[0] - y[1];
            f32 denom21 = y[2] - y[1];
            Vec2f dir20; compute_dir(dir20, xz[2], xz[0], denom20);
            Vec2f dir01; compute_dir(dir01, xz[0], xz[1], denom01);
            Vec2f dir21; compute_dir(dir21, xz[2], xz[1], denom21);
            Vec2f polygon[5];
            compute_polygon_point(polygon[0], xz[0], numerNeg0, dir20);
            compute_polygon_point(polygon[1], xz[1], numerNeg1, dir01);
            compute_polygon_point(polygon[2], xz[1], 0, gVec2fZero);
            compute_polygon_point(polygon[3], xz[1], numerPos1, dir21);
            compute_polygon_point(polygon[4], xz[0], numerPos0, dir20);
            return disk_overlaps_polygon(polygon, 5, radius2);
        }

        // Highest is above the bottom of the cylinder
        if (y[2] > -heightHalf) {

            // Middle is at the bottom or below the cylinder
            if (y[1] <= -heightHalf) {
                f32 numerNeg2 = -heightHalf - y[2];
                f32 denom02 = y[0] - y[2];
                f32 denom12 = y[1] - y[2];
                Vec2f dir02; compute_dir(dir02, xz[0], xz[2], denom02);
                Vec2f dir12; compute_dir(dir12, xz[1], xz[2], denom12);
                Vec2f polygon[3];
                compute_polygon_point(polygon[0], xz[2], 0, gVec2fZero);
                compute_polygon_point(polygon[1], xz[2], numerNeg2, dir02);
                compute_polygon_point(polygon[2], xz[2], numerNeg2, dir12);
                return disk_overlaps_polygon(polygon, 3, radius2);
            }

            // Middle is above the bottom of the cylinder
            f32 numerNeg0 = -heightHalf - y[0];
            f32 denom10 = y[1] - y[0];
            f32 denom20 = y[2] - y[0];
            Vec2f dir20; compute_dir(dir20, xz[2], xz[0], denom20);
            Vec2f dir10; compute_dir(dir10, xz[1], xz[0], denom10);
            Vec2f polygon[4];
            compute_polygon_point(polygon[0], xz[0], numerNeg0, dir20);
            compute_polygon_point(polygon[1], xz[0], numerNeg0, dir10);
            compute_polygon_point(polygon[2], xz[1], 0, gVec2fZero);
            compute_polygon_point(polygon[3], xz[2], 0, gVec2fZero);
            return disk_overlaps_polygon(polygon, 4, radius2);
        }

        // Middle is below the cylinder
        if (y[1] < -heightHalf) {
            return disk_overlaps_point(xz[2], radius2);
        }

        // Middle and highest are at the bottom of the cylinder
        return disk_overlaps_segment(xz[1], xz[2], radius2);
    }

    // Lowest is below the top of the cylinder
    if (y[0] < heightHalf) {

        // Middle is at the top or above the cylinder
        if (y[1] >= heightHalf) {
            f32 numerPos0 = +heightHalf - y[0];
            f32 denom10 = y[1] - y[0];
            f32 denom20 = y[2] - y[0];
            Vec2f dir10; compute_dir(dir10, xz[1], xz[0], denom10);
            Vec2f dir20; compute_dir(dir20, xz[2], xz[0], denom20);
            Vec2f polygon[3];
            compute_polygon_point(polygon[0], xz[0], 0, gVec2fZero);
            compute_polygon_point(polygon[1], xz[0], numerPos0, dir10);
            compute_polygon_point(polygon[2], xz[0], numerPos0, dir20);
            return disk_overlaps_polygon(polygon, 3, radius2);
        }

        // Highest and middle are at the top or above the cylinder
        f32 numerPos2 = +heightHalf - y[2];
        f32 denom02 = y[0] - y[2];
        f32 denom12 = y[1] - y[2];
        Vec2f dir02; compute_dir(dir02, xz[0], xz[2], denom02);
        Vec2f dir12; compute_dir(dir12, xz[1], xz[2], denom12);
        Vec2f polygon[4];
        compute_polygon_point(polygon[0], xz[0], 0, gVec2fZero);
        compute_polygon_point(polygon[1], xz[1], 0, gVec2fZero);
        compute_polygon_point(polygon[2], xz[2], numerPos2, dir12);
        compute_polygon_point(polygon[3], xz[2], numerPos2, dir02);
        return disk_overlaps_polygon(polygon, 4, radius2);
    }

    // Middle is above the cylinder
    if (y[1] > heightHalf) {
        return disk_overlaps_point(xz[0], radius2);
    }

    // Lowest and middle are at the top of the cylinder
    return disk_overlaps_segment(xz[0], xz[1], radius2);
}

struct Surface *get_pseudo_floor_at_pos(f32 x, f32 y, f32 z) {
    static struct Surface sPseudoFloors[2];
    struct Surface *pseudoFloor = &sPseudoFloors[gOmmGlobals->cameraUpdate];
    pseudoFloor->type = SURFACE_DEFAULT;
    pseudoFloor->force = 0;
    pseudoFloor->flags = 0;
    pseudoFloor->room = -1;
    pseudoFloor->lowerY = y;
    pseudoFloor->upperY = y;
    pseudoFloor->vertex1[0] = x;
    pseudoFloor->vertex1[1] = y;
    pseudoFloor->vertex1[2] = z;
    pseudoFloor->vertex2[0] = x;
    pseudoFloor->vertex2[1] = y;
    pseudoFloor->vertex2[2] = z;
    pseudoFloor->vertex3[0] = x;
    pseudoFloor->vertex3[1] = y;
    pseudoFloor->vertex3[2] = z;
    pseudoFloor->normal.x = 0.f;
    pseudoFloor->normal.y = 1.f;
    pseudoFloor->normal.z = 0.f;
    pseudoFloor->originOffset = -y;
    pseudoFloor->object = NULL;
    return pseudoFloor;
}

Vec3fp omm_vec3f_rotate_zxy(Vec3f dest, Vec3f v, s16 pitch, s16 yaw, s16 roll) {
    Vec3s r = { pitch, yaw, roll };
    Mat4 mtx;
    mtxf_rotate_zxy_and_translate(mtx, gVec3fZero, r);
    f32 x = v[0];
    f32 y = v[1];
    f32 z = v[2];
    dest[0] = x * mtx[0][0] + y * mtx[1][0] + z * mtx[2][0] + mtx[3][0];
    dest[1] = x * mtx[0][1] + y * mtx[1][1] + z * mtx[2][1] + mtx[3][1];
    dest[2] = x * mtx[0][2] + y * mtx[1][2] + z * mtx[2][2] + mtx[3][2];
    return dest;
}
// Original OMM raycast helper; coopdx only exposes find_collisions_on_ray.
bool find_first_hit_on_ray(Vec3f orig, Vec3f dir, Vec3f hitPos, f32 offset, f32 surfaceScale, u32 flags) {
    RayCollisionData hits;
    if (find_collisions_on_ray(orig, dir, &hits, surfaceScale, flags)) {
        const RayHit *hit = hits.hits;
        hitPos[0] = hit->pos[0] + offset * hit->surf->normal.x;
        hitPos[1] = hit->pos[1] + offset * hit->surf->normal.y;
        hitPos[2] = hit->pos[2] + offset * hit->surf->normal.z;
        return true;
    }
    vec3f_sum(hitPos, orig, dir);
    return false;
}

//
// FS helpers from the original OMM port.
//

// Complete the opaque fs_dir_t definition used by fs_file_t->parent.
struct fs_dir_s {
    void *pack;
    const char *realpath;
    fs_packtype_t *packer;
    struct fs_dir_s *prev, *next;
};

bool fs_seek(fs_file_t *file, s64 ofs) {
    if (file == NULL || file->parent == NULL || file->parent->packer == NULL || file->parent->packer->seek == NULL) {
        return false;
    }
    return file->parent->packer->seek(file->parent->pack, file, (int64_t) ofs);
}

s64 fs_fsize(const char *vpath) {
    fs_file_t *file = fs_open(vpath);
    if (file == NULL) { return -1; }
    s64 size = fs_size(file);
    fs_close(file);
    return size;
}

const char *fs_cat_paths(sys_path_t dst, const char *path1, const char *path2) {
    if (path1 == NULL) { path1 = ""; }
    if (path2 == NULL) { path2 = ""; }
    snprintf(dst, sizeof(sys_path_t), "%s%s", path1, path2);
    return dst;
}
