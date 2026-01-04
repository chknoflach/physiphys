#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MEM_STEP 64
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
    Vector2         v;          // m/s
    Vector2         pos;        // meters
    Vector3         dim;        // dimensions as bounding box
    bool            settled;    // is this body at rest?
    bool            stationary; // marks as immovable
    ph_material     material;
} ph_body;

typedef struct {
    ph_body  *items;
    size_t   count;
    size_t   capacity;
} ph_collection;

typedef struct {
    float   g;
    size_t  g_px;
    size_t  fps;
    float   dt;
    float   dt_acc;
} ph_settings;

typedef struct {
    int     width;
    int     height;
    int     fps;
} gfx_settings;

static const ph_body DEFAULT_BODY = {
    .mass = 1.0f,
    .drag = 0.2f,
    .g_tweak = 1.0f,
    .pos.x = 0.0f,
    .pos.y = 0.0f,
    .v.x = 0.0f,
    .v.y = 0.0f,
    .dim.x = 1.0f,
    .dim.y = 1.0f,
    .dim.z = 1.0f,
    .settled = false,
    .stationary = false,
    .material.color = RED,
    .material.hard = 1.0f,
    .material.smooth = 1.0f,
    .material.elastic = 1.0f
};

ph_body     create_ph_body(float, float, float, float, float);
void        add_to_collection(ph_collection *, ph_body);
void        remove_from_collection(ph_collection *, size_t);
void        update_physics(gfx_settings *, ph_settings *, ph_collection *, float);
int         detect_collission(ph_body *, ph_body *);
void        draw_collection(ph_collection *);
int         sort_ph_collection_comp(const void *, const void *);
void        _reset(ph_collection *);

int main(void)
{
    gfx_settings gfx = {0};
    ph_settings phys = {0};
    ph_collection bodies = {0};
    float dt;
    
    gfx.width = 800;
    gfx.height = 600;
    gfx.fps = 720;

    phys.g = 9.81f;
    phys.g_px = phys.g * 200.0f;
    phys.fps = 240;
    phys.dt = 1.0f / phys.fps;
    
    InitWindow(gfx.width, gfx.height, "raylib test");
    SetTargetFPS(gfx.fps);

    ph_body ground = create_ph_body(0.0f, 0.0f, 400.0f, 20.0f, 1.0f);
    ground.pos.x = (gfx.width - 400.0f) / 2;
    ground.pos.y = (gfx.height - 20.0f);
    ground.stationary = true;
    ground.material.color = SKYBLUE;
    add_to_collection(&bodies, ground);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        if (IsKeyPressed(KEY_R))
            _reset(&bodies);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ph_body bd = create_ph_body(2.0f, 0.2f, 10.0f, 10.0f, 10.0f);
            bd.pos = GetMousePosition();
            bd.pos.x = floorf(bd.pos.x);
            bd.pos.y = floorf(bd.pos.y);
            bd.drag = (10 * ((float)rand() / RAND_MAX));
            add_to_collection(&bodies, bd);
        }
        update_physics(&gfx, &phys, &bodies, dt);
        BeginDrawing();
        ClearBackground(BLACK);
        draw_collection(&bodies);
        DrawText(TextFormat("FPS: %08f", 1/dt), 10, 10, 10, WHITE);
        EndDrawing();
    }
    CloseWindow();
    return (0);
}

int detect_collission(ph_body *a, ph_body *b)
{
    if (a->pos.x + a->dim.x < b->pos.x || b->pos.x + b->dim.x < a->pos.x)
        return (0);
    if (a->pos.y + a->dim.y < b->pos.y || b->pos.y + b->dim.y < a->pos.y)
        return (0);
    return (1);
}

void draw_collection(ph_collection *c)
{
    for (size_t i = 0; i < c->count; i++)
    {
        ph_body *bd = &(c->items[i]);
        DrawRectangleLines(bd->pos.x, bd->pos.y, bd->dim.x, bd->dim.y,
                bd->material.color);
    }
}

void _reset(ph_collection *c)
{
    c->count = 0;
    c->capacity = 0;
    free(c->items); 
    c->items = NULL;
}

int sort_ph_collection_comp(const void *a, const void *b)
{
    float ca = ((ph_body*)a)->pos.y + ((ph_body*)a)->dim.y;
    float cb = ((ph_body*)b)->pos.y + ((ph_body*)b)->dim.y;

    return ((ca > cb) - (ca < cb));
}

void update_physics(gfx_settings *gfx, ph_settings *phy,
        ph_collection *c, float dt)
{
    phy->dt_acc += dt;

    while (phy->dt_acc >= phy->dt)
    {
        qsort(c->items, c->count, sizeof(*c->items),
                sort_ph_collection_comp);
        for (size_t i = 0; i < c->count; i++)
        {
            ph_body    *bd = &(c->items[i]);

            if (bd->settled || bd->stationary)
                continue;

            if (bd->pos.y >= (float)gfx->height || bd->pos.x >= (float)gfx->width)
            {
                remove_from_collection(c, i);
                continue;
            }

            float   acc_y = 0.0f; 
            int     ix = (int)floorf(bd->pos.x);
            int     iw = (int)ceilf(bd->dim.x);
            bool    collided;
            ph_body *o;

            if (ix >= gfx->width || ix + iw < 0) continue;
            if (ix < 0) ix = 0;
            if (iw + ix >= gfx->width) iw = gfx->width - ix;

            for (size_t j = 0; j < c->count; j++)
            {
                if (j == i) // skip self
                    continue;
                o = &(c->items[j]);
                collided = detect_collission(bd, o);
                if (!collided)
                    continue;
                if (bd->stationary || bd->settled)
                {
                    o->v.y = 0.0f;
                    o->settled = true;
                    continue;
                }
                if (o->stationary || o->settled)
                {
                    bd->v.y = 0.0f;
                    bd->settled = true;
                    continue;
                }
                bd->pos.y = o->pos.y > bd->pos.y ? o->pos.y - bd->dim.y : bd->pos.y;
                o->pos.y  = o->pos.y < bd->pos.y ? bd->pos.y - o->dim.y : o->pos.y;
                float v = (bd->mass * bd->v.y + o->mass * o->v.y)
                        / (bd->mass + o->mass);
                bd->v.y = v;
                o->v.y = v;
            }
            if (false) // TODO: collision
            {
                bd->v.y = 0.0f;
                bd->settled = true;
            }
            else
            {
                acc_y = (phy->g_px * bd->g_tweak) -
                    (bd->drag / bd->mass) * bd->v.y * fabsf(bd->v.y);
                bd->v.y += acc_y * phy->dt;
                bd->pos.y += bd->v.y * phy->dt;
            }
        }
        phy->dt_acc -= phy->dt;
    }
}

void    add_to_collection(ph_collection *cl, ph_body bd)
{
    if (!cl->capacity || cl->count >= cl->capacity)
    {
        cl->items = realloc(cl->items, sizeof(bd) * (cl->capacity + MEM_STEP));
        cl->capacity += MEM_STEP;
    }
    cl->items[cl->count] = bd;
    cl->count++;
}

void    remove_from_collection(ph_collection *cl, size_t index)
{
    cl->count--;

    for (size_t i = index; i < cl->count; i++)
        cl->items[i] = cl->items[i + 1];
}

ph_body    create_ph_body(float mass, float drag, float x, float y, float z)
{
    ph_body    bd;

    bd = DEFAULT_BODY;
    bd.mass = mass;
    bd.drag = drag;
    bd.dim.x = x;
    bd.dim.y = y;
    bd.dim.z = z;
    return (bd);
}

