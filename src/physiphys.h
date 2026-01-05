#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#define MEM_STEP 64

#define FLAG_LOG            (1u << 0)

#define DEFER_RESET         (1u << 0)
#define DEFER_SIZE_PLUS     (1u << 1)
#define DEFER_SIZE_MINUS    (1u << 2)

/*    Color colors[MAX_COLORS_COUNT] = {
        DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN,
        GRAY, RED, GOLD, LIME, BLUE, VIOLET, BROWN, LIGHTGRAY, PINK, YELLOW,
        GREEN, SKYBLUE, PURPLE, BEIGE };
 */
typedef struct {
    Color   color;
    float   hard;
    float   smooth;
    float   elastic;
} ph_material;

typedef struct {
    float           mass;       // mass in kg
    float           drag;       // drag coefficient
    float           g_tweak;    // manual gravity tweak; 1.0f = normal
    Vector3         v;          // m/s
    Vector3         pos;        // meters
    Vector3         dim;        // dimensions as bounding box
    bool            settled;    // is this body at rest?
    bool            stationary; // marks as immovable
    ph_material     material;
} ph_body;

typedef struct {
    size_t  active;
    size_t  collissions;
    size_t  dropped;
} ph_stats;

typedef struct {
    ph_body *items;
    size_t  count;
    size_t  capacity;
} ph_collection;

typedef struct {
    ph_body *og;
    ph_body *cl;
    size_t  i;
} ph_collission;

typedef struct {
    float   g;
    size_t  g_px;
    size_t  fps;
} ph_settings;

typedef struct {
    int     width;
    int     height;
    int     fps;
} gfx_settings;

typedef struct {
    gfx_settings    gfx;
    ph_settings     ph;
    ph_collection   c;
    size_t          flags;
    size_t          defer;
    float           size;
    float           dt;
    float           ups;
    float           dt_draw;
    float           dt_phys;
    float           dt_game;
    ph_stats        stats;
} game_state;

ph_body         create_ph_body(float, float, float, float, float);
void            add_to_collection(ph_collection *, ph_body);
void            remove_from_collection(ph_collection *, size_t);
void            update_inputs(game_state *);
void            update_game(game_state *);
void            update_physics(game_state *);
void            update_draw(game_state *);
ph_body         create_body_xy(Vector2, Vector3);
void            spawn_physics_body(ph_collection *, ph_body);
int             detect_collission(ph_body *, ph_body *);
ph_collission   get_collission(ph_collection *, ph_body *, size_t);
bool            resolve_collission(ph_collission);
void            draw_collection(ph_collection *);
void            draw_log(game_state *);
int             sort_ph_collection_comp(const void *, const void *);
void            _reset(game_state *);

