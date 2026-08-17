#ifndef OMM_SURFACE_DATA_H
#define OMM_SURFACE_DATA_H

#include "types.h"
#include "behavior_data.h"
#include "game/object_list_processor.h"

struct SurfaceData {
    bool dynamic;
    union {
        struct Surface *surface;
        struct {
            struct Object *object;
            const BehaviorScript *behavior;
            const void *collision;
            s32 index;
            Vec3f pos;
            Vec3s angle;
            Vec3f scale;
        };
    };
};

void clear_surface_data(struct SurfaceData *data);
void get_surface_data(struct SurfaceData *data, struct Surface *surf);
struct Surface *get_surface_from_data(const struct SurfaceData *data);
struct Surface *get_next_surface(struct Surface *surf);

void omm_load_object_collision_model(void);
void omm_surface_add_all_surfaces_again(void);
void omm_debug_surfaces_print_info(s32 x, s32 y);

#endif