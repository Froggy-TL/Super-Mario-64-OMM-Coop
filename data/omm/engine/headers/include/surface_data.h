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

#endif