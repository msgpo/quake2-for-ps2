
/* ================================================================================================
 * -*- C -*-
 * File: model.h
 * Author: Guilherme R. Lampert
 * Created on: 29/10/15
 * Brief: Declarations related to 3D model loading and handling for entities and world/level.
 *        Structured found here were mostly adapted from 'ref_gl/gl_model.h'
 *
 * This source code is released under the GNU GPL v2 license.
 * Check the accompanying LICENSE file for details.
 * ================================================================================================ */

#ifndef PS2_MODEL_H
#define PS2_MODEL_H

#include "ps2/ref_ps2.h"

enum
{
    // Plane sides:
    SIDE_FRONT          = 0,
    SIDE_BACK           = 1,
    SIDE_ON             = 2,

    // Misc surface flags (same values used by ref_gl):
    SURF_PLANEBACK      = 2,
    SURF_DRAWSKY        = 4,
    SURF_DRAWTURB       = 16,
    SURF_DRAWBACKGROUND = 64,
    SURF_UNDERWATER     = 128,

    // Number of element in the poly vertex.
    POLY_VERTEX_SIZE    = 7
};

/*
==============================================================

In-memory representation of 3D models (world and entities):

==============================================================
*/

/*
 * Model vertex position:
 */
typedef struct ps2_mdl_vertex_s
{
    vec3_t position;
} ps2_mdl_vertex_t;

/*
 * Sub-model mesh data:
 */
typedef struct ps2_mdl_sub_s
{
    vec3_t mins;
    vec3_t maxs;
    vec3_t origin;
    float radius;
    int head_node;
    int vis_leafs;
    int first_face;
    int num_faces;
} ps2_mdl_sub_t;

/*
 * Edge description:
 */
typedef struct ps2_mdl_edge_s
{
    u16 v[2]; // vertex numbers
    u32 cached_edge_offset;
} ps2_mdl_edge_t;

/*
 * Texture/material description:
 */
typedef struct ps2_mdl_texinfo_s
{
    float vecs[2][4];
    int flags;
    int num_frames;
    ps2_teximage_t * teximage;
    struct ps2_mdl_texinfo_s * next; // animation chain
} ps2_mdl_texinfo_t;

/*
 * Model polygon/face:
 * List links are for draw sorting.
 */
typedef struct ps2_mdl_poly_s
{
    struct ps2_mdl_poly_s * next;
    struct ps2_mdl_poly_s * chain;
    int flags;                        // for SURF_UNDERWATER (not needed anymore?)
    int num_verts;
    float verts[4][POLY_VERTEX_SIZE]; // variable sized (xyz s1t1 s2t2)
} ps2_mdl_poly_t;

/*
 * Surface description:
 * (holds a set of polygons)
 */
typedef struct ps2_mdl_surface_s
{
    int vis_frame; // should be drawn when node is crossed

    cplane_t * plane;
    int flags;

    int first_edge; // look up in model->surfedges[], negative numbers
    int num_edges;  // are backwards edges

    s16 texture_mins[2];
    s16 extents[2];

    int light_s, light_t;   // lightmap tex coordinates
    int dlight_s, dlight_t; // lightmap tex coordinates for dynamic lightmaps

    ps2_mdl_poly_t * polys; // multiple if warped
    struct ps2_mdl_surface_s * texture_chain;
    struct ps2_mdl_surface_s * lightmap_chain;

    ps2_mdl_texinfo_t * texinfo;

    // dynamic lighting info:
    int dlight_frame;
    int dlight_bits;

    int lightmap_texture_num;
    byte styles[MAXLIGHTMAPS];
    float cached_light[MAXLIGHTMAPS]; // values currently used in lightmap
    byte * samples;                   // [numstyles*surfsize]
} ps2_mdl_surface_t;

/*
 * BSP world node:
 */
typedef struct ps2_mdl_node_s
{
    // common with leaf
    int contents;  // -1, to differentiate from leafs
    int vis_frame; // node needs to be traversed if current

    // for bounding box culling
    float minmaxs[6];

    struct ps2_mdl_node_s * parent;

    // node specific
    cplane_t * plane;
    struct ps2_mdl_node_s * children[2];

    u16 first_surface;
    u16 num_surfaces;
} ps2_mdl_node_t;

/*
 * Special BSP leaf node (draw node):
 */
typedef struct ps2_mdl_leaf_s
{
    // common with node
    int contents;  // will be a negative contents number
    int vis_frame; // node needs to be traversed if current

    // for bounding box culling
    float minmaxs[6];

    struct ps2_mdl_node_s * parent;

    // leaf specific
    int cluster;
    int area;

    ps2_mdl_surface_t ** first_mark_surface;
    int num_mark_surfaces;
} ps2_mdl_leaf_t;

/*
 * Misc model type flags:
 */
typedef enum
{
    MDL_BRUSH  = (1 << 1),
    MDL_SPRITE = (1 << 2),
    MDL_ENTITY = (1 << 3)
} ps2_mdl_type_t;

/*
==============================================================

Whole model (world or entity/sprite):

==============================================================
*/

typedef struct ps2_model_s
{
    ps2_mdl_type_t type;
    int num_frames;
    int flags;

    // Volume occupied by the model graphics.
    vec3_t mins;
    vec3_t maxs;
    float radius;

    // Solid volume for clipping.
    qboolean clipbox;
    vec3_t clipmins;
    vec3_t clipmaxs;

    // Brush model.
    int first_model_surface;
    int num_model_surfaces;
    int lightmap; // Only for submodels

    int num_submodels;
    ps2_mdl_sub_t * submodels;

    int num_planes;
    cplane_t * planes;

    int num_leafs; // Number of visible leafs, not counting 0
    ps2_mdl_leaf_t * leafs;

    int num_vertexes;
    ps2_mdl_vertex_t * vertexes;

    int num_edges;
    ps2_mdl_edge_t * edges;

    int num_nodes;
    int first_node;
    ps2_mdl_node_t * nodes;

    int num_texinfos;
    ps2_mdl_texinfo_t * texinfos;

    int num_surfaces;
    ps2_mdl_surface_t * surfaces;

    int num_surf_edges;
    int * surf_edges;

    int num_mark_surfaces;
    ps2_mdl_surface_t ** mark_surfaces;

    dvis_t * vis;
    byte   * light_data;

    // For alias models and skins.
    ps2_teximage_t * skins[MAX_MD2SKINS];

    // Registration number, so we know if it is currently referenced by the level being played.
    int registration_sequence;

    // Memory hunk backing the model's data.
    mem_hunk_t hunk;

    // Hash of the following name string, for faster lookup.
    u32 hash;

    // File name with path.
    char name[MAX_QPATH];
} ps2_model_t;

/*
==============================================================

Public model loading/management functions:

==============================================================
*/

// Global initialization/shutdown:
void PS2_ModelInit(void);
void PS2_ModelShutdown(void);

// Allocate a blank model from the pool.
// Fails with a Sys_Error if no more slots are available.
ps2_model_t * PS2_ModelAlloc(void);

// Looks up an already loaded model or tries to load it from disk for the first time.
ps2_model_t * PS2_ModelFindOrLoad(const char * name, int flags);

// Loads the world model used by the current level the game wants.
// The returned pointer points to an internal shared instance, so
// only one world model is allowed at any time.
ps2_model_t * PS2_ModelLoadWorld(const char * name);

// Frees a model previously acquired from FindOrLoad.
void PS2_ModelFree(ps2_model_t * mdl);

#endif // PS2_MODEL_H
