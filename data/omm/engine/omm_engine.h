#ifndef OMM_ENGINE_H
#define OMM_ENGINE_H

#include "animation.h"
#include "PR/gbi.h"
#include "engine/math_util.h"
#include "engine/geo_layout.h"
#include "engine/graph_node.h"
#include "headers/src/engine/graph_node_o.h"
#include "engine/behavior_script.h"
#include "engine/level_script.h"
#include "engine/surface_load.h"
#include "engine/surface_collision.h"
#include "game/rendering_graph_node.h"
#include "game/save_file.h"
#include "game/shadow.h"
#include "menu/file_select.h"

//
// Surface Pool
//

typedef struct { OmmArray data; s32 count; } OmmSurfaceArray;
typedef OmmSurfaceArray OmmSurfaceHashMap[NUM_CELLS][NUM_CELLS][3];
extern OmmSurfaceHashMap gOmmSurfaces[2];

#define OMM_NUM_SURFACE_ARRAYS  (sizeof(gOmmSurfaces) / sizeof(OmmSurfaceArray)) // (2 * NUM_CELLS * NUM_CELLS * 3)

#define gOmmFloors(dyn, cx, cz) (&gOmmSurfaces[dyn][cz][cx][SURFACE_FLOOR])
#define gOmmCeils(dyn, cx, cz)  (&gOmmSurfaces[dyn][cz][cx][SURFACE_CEIL])
#define gOmmWalls(dyn, cx, cz)  (&gOmmSurfaces[dyn][cz][cx][SURFACE_WALL])

#endif
