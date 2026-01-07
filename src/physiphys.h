#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdint.h>
#include "raylib.h"

#define MEM_STEP            64
#define COLLISION_PASSES    4

typedef uint32_t update_flags;
enum {
    UPDATE_FLAG_NONE         = (0),
    UPDATE_FLAG_RESET        = (1u << 0),
    UPDATE_FLAG_SIZEYUP      = (1u << 1),
    UPDATE_FLAG_SIZEXUP      = (1u << 2),
    UPDATE_FLAG_SIZEYDN      = (1u << 3),
    UPDATE_FLAG_SIZEXDN      = (1u << 4)
};

typedef uint32_t ph_flags;
enum {
    PH_FLAG_NONE            = (0),
    PH_FLAG_SETTLED         = (1u << 0)
};

typedef uint32_t draw_flags;
enum {
    DRAW_FLAG_NONE          = (0),
    DRAW_FLAG_LOG           = (1u << 0)
};

typedef enum {
    PROJECTION_PERSPECTIVE,
    PROJECTION_ORTHOGRAPHIC
} projection;

typedef uint32_t entity_id;

#define ENTID_INDEX_BITS    20
#define ENTID_GEN_BITS      12
#define ENTID_INDEX_MASK    ((1u << ENTID_INDEX_BITS) - 1)

typedef struct {
    Vector3 v;
    Vector3 pos;
} ph_state;

typedef struct {
    uint8_t _placeholder;
} ph_material;

typedef struct {
    float               inv_mass;
    float               drag;
    Vector3             size;
    ph_state            cur;
    ph_state            prev;
    ph_flags            flags;
    const ph_material   *material;
    Vector3             force;
} ph_body;

typedef uint32_t dense_i;
typedef struct {
    dense_i from;
    dense_i to;
    Vector3 normal;
    float   penetration;
} ph_contact;

typedef struct {
    Color   color;
} gfx_sprite;

typedef struct {
    gfx_sprite  sprite;
} gfx_comp;

#define SPA_INVALID_INDEX UINT32_MAX

typedef struct {
    ph_body     *dense;
    entity_id   *dense_e;
    uint32_t    *sparse;
    uint32_t    count;
    uint32_t    cap;
} ph_spa;

typedef struct {
    gfx_comp    *dense;
    entity_id   *dense_e;
    uint32_t    *sparse;
    uint32_t    count;
    uint32_t    cap;
} gfx_spa;

typedef struct {
    uint32_t    next_index;

    uint32_t    *re;
    uint32_t    re_count;
    uint32_t    re_cap;
    
    uint16_t    *gen;
    uint32_t    gen_cap;
} id_pool;

typedef struct {
    id_pool pool;
    ph_spa      ph;
    gfx_spa     render;
} scene;

typedef struct {
    float   g;
    int     hz;
} ph_settings;

typedef struct {
    int     width;
    int     height;
    int     fps;
} gfx_settings;

typedef struct {
    Vector3     position;
    Vector3     target;
    Vector3     up;
    float       fovy;
    projection  projection;
} gfx_cam_3d;

typedef struct {
    float   acc;
    float   step;
} time_step;

typedef struct {
    struct {
        gfx_settings    gfx;
        ph_settings     ph;
    } cfg;
    struct {
        update_flags    update;
        draw_flags      draw;
    } flags;
    struct {
        size_t          active;
        size_t          collisions;
        size_t          dropped;
    } stats;
    struct {
        Vector2         mouse_prev;
    } input;
    time_step           ph_clock;
    scene               active_scene;
} game_state;


entity_id   entid_make(uint32_t index, uint32_t gen);
uint32_t    entid_index(entity_id id);
uint32_t    entid_gen(entity_id id);

void        id_pool_init(id_pool *p, uint32_t initial_cap);
void        id_pool_free(id_pool *p);

entity_id   entity_create(id_pool *p);
void        entity_destroy(id_pool *p, entity_id id);
bool        entity_alive(const id_pool *p, entity_id id);

void        ph_spa_init(ph_spa *s, uint32_t dense_cap, uint32_t sparse_cap);
void        ph_spa_free(ph_spa *s);

bool        ph_spa_has(const ph_spa *s, entity_id e);
ph_body*    ph_spa_get(ph_spa *s, entity_id e);

ph_body*    ph_spa_add(ph_spa *s, entity_id e, ph_body value);
bool        ph_spa_remove(ph_spa *s, entity_id e);

entity_id   ph_spa_entity_at(const ph_spa *s, dense_i di);
ph_body*    ph_spa_dense_at(ph_spa *s, dense_i di);


void        gfx_spa_init(gfx_spa *s, uint32_t dense_cap, uint32_t sparse_cap);
void        gfx_spa_free(gfx_spa *s);

bool        gfx_spa_has(const gfx_spa *s, entity_id e);
gfx_comp*   gfx_spa_get(gfx_spa *s, entity_id e);

gfx_comp*   gfx_spa_add(gfx_spa *s, entity_id e, gfx_comp value);
bool        gfx_spa_remove(gfx_spa *s, entity_id e);


void        scene_init(scene *sc, uint32_t entity_cap);
void        scene_free(scene *sc);

entity_id   scene_create_entity(scene *sc);
void        scene_destroy_entity(scene *sc, entity_id e);

ph_body*    scene_add_ph(scene *sc, entity_id e, ph_body body);
gfx_comp*   scene_add_gfx(scene *sc, entity_id e, gfx_comp comp);

ph_body*    scene_get_ph(scene *sc, entity_id e);
gfx_comp*   scene_get_gfx(scene *sc, entity_id e);

bool        scene_remove_ph(scene *sc, entity_id e);
bool        scene_remove_gfx(scene *sc, entity_id e);


bool        ph_aabb_overlap(const ph_body *a, const ph_body *b);
bool        ph_contact_build(const ph_body *a, const ph_body *b, ph_contact *out);
bool        ph_contact_resolve(scene *sc, ph_contact c);


void        game_init(game_state *st);
void        game_reset(game_state *st);

void        game_scan_input(game_state *st);
void        game_fixed_update(game_state *st, float step); // apply deferred ops + spawning, etc.

void        game_physics_step(game_state *st, float step);
void        game_render(const game_state *st, float alpha);

void        game_draw_log(const game_state *st);

ph_body     ph_body_default(void);
ph_body     ph_body_make_box(Vector3 pos, Vector3 size, Vector3 v, float inv_mass, float drag);
void        ph_body_apply_force(ph_body *b, Vector3 f);

entity_id   scene_spawn_box(scene *sc, ph_body body, gfx_comp gfx);
