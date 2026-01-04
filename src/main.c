#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

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
    ph_body *items;
    size_t  count;
    size_t  capacity;
    size_t  active;
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

ph_body         create_ph_body(float, float, float, float, float);
void            add_to_collection(ph_collection *, ph_body);
void            remove_from_collection(ph_collection *, size_t);
void            update_physics(gfx_settings *, ph_settings *, ph_collection *, float);
int             detect_collission(ph_body *, ph_body *);
ph_collission   get_collission(ph_collection *, ph_body *, size_t);
void            draw_collection(ph_collection *);
int             sort_ph_collection_comp(const void *, const void *);
void            _reset(ph_collection *, gfx_settings *);

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
    
    SetTargetFPS(gfx.fps);
    InitWindow(gfx.width, gfx.height, "raylib test");
    _reset(&bodies, &gfx);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        if (IsKeyPressed(KEY_R))
            _reset(&bodies, &gfx);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            float   tmp_d = 5.0f;
            ph_collission check;
            ph_body bd = create_ph_body(2.0f, 0.2f, tmp_d, tmp_d, tmp_d);
            bd.pos = GetMousePosition();
            bd.pos.x = floorf(bd.pos.x) - bd.dim.x / 2;
            bd.pos.y = floorf(bd.pos.y) - bd.dim.y / 2;
            bd.drag = ((float)rand() / RAND_MAX);
            check = get_collission(&bodies, &bd, 0);
            if (check.cl == 0)
                add_to_collection(&bodies, bd);
        }
        update_physics(&gfx, &phys, &bodies, dt);
        BeginDrawing();
        ClearBackground(BLACK);
        draw_collection(&bodies);

        float effps = GetFPS();
        Color fpsc = GREEN;
        if (effps / gfx.fps < 0.9)
            fpsc = YELLOW;
        if (effps / gfx.fps < 0.6)
            fpsc = ORANGE;
        if (effps / gfx.fps < 0.4)
            fpsc = RED;
        DrawText(TextFormat("FPS: %d", (int)(effps)), 10, 10, 10, fpsc);
        DrawText(TextFormat("Phys Entities:"), 10, 30, 10, LIGHTGRAY);
        DrawText(TextFormat("Total: %ld", bodies.count), 10, 45, 10, LIGHTGRAY);
        DrawText(TextFormat("Active: %d", bodies.active), 10, 60, 10, LIGHTGRAY);
        EndDrawing();
    }
    CloseWindow();
    return (0);
}

int detect_collission(ph_body *a, ph_body *b)
{
    if (a == b)
        return (0);
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

void _reset(ph_collection *c, gfx_settings *gfx)
{
    c->count = 0;
    c->capacity = 0;
    free(c->items); 
    c->items = NULL;

    ph_body ground = create_ph_body(0.0f, 0.0f, 400.0f, 20.0f, 1.0f);
    ground.pos.x = (gfx->width - 400.0f) / 2;
    ground.pos.y = (gfx->height - 20.0f);
    ground.stationary = true;
    ground.material.color = SKYBLUE;
    add_to_collection(c, ground);
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
        c->active = 0;
        for (size_t i = 0; i < c->count; i++)
        {
            ph_body    *bd = &(c->items[i]);
            // prune out of of bounds elements
            while( (i < c->count) && 
                   (   (bd->pos.y >= (float)gfx->height && bd->v.y >= 0)
                    || (bd->pos.y <= 0.0f - bd->dim.y && bd->v.y <= 0) 
                    || (bd->pos.x >= (float)gfx->width && bd->v.x >= 0)
                    || (bd->pos.x <= 0.0f - bd->dim.x && bd->v.x <= 0)))
            {
                remove_from_collection(c, i);
                bd = &(c->items[i]);
            }
            // skip elements at rest
            if (bd->settled || bd->stationary)
                continue;
            c->active++;
            float   acc_y = 0.0f; 
            int     ix = (int)floorf(bd->pos.x);
            int     iw = (int)ceilf(bd->dim.x);

            if (ix < 0) {
                iw += ix;
                ix = 0;
            }
            if (iw + ix >= gfx->width) iw = gfx->width - ix;

            // since we skip elements at rest, we need to check
            // collissions against all elements. If we would not skip
            // elements at rest, we could check only forward.
            ph_collission check = get_collission(c, bd, 0);
            if (check.cl)
            {
                if (check.og->stationary || check.og->settled
                 || check.cl->stationary || check.cl->settled)
                {
                    check.og->v.y = check.cl->v.y = 0.0f;
                    check.og->settled = check.cl->settled = true;
                    continue;
                }
                // TODO: how/wether to handle cascading collissions due
                // to changing position; currently, we just implicitly defer
                // to the next physics frame
                check.og->pos.y = check.cl->pos.y > check.og->pos.y ?
                    check.cl->pos.y - check.og->dim.y : check.og->pos.y;
                check.cl->pos.y  = check.cl->pos.y < check.og->pos.y ?
                    check.og->pos.y - check.cl->dim.y : check.cl->pos.y;
                float v = (check.og->mass * check.og->v.y
                            + check.cl->mass * check.cl->v.y)
                        / (check.og->mass + check.cl->mass);
                check.og->v.y = v;
                check.cl->v.y = v;
            }
            acc_y = (phy->g_px * bd->g_tweak) -
                (bd->drag / bd->mass) * bd->v.y * fabsf(bd->v.y);
            bd->v.y += acc_y * phy->dt;
            bd->pos.y += bd->v.y * phy->dt;
        }
        phy->dt_acc -= phy->dt;
    }
}

ph_collission get_collission(ph_collection *cl, ph_body *bd, size_t i)
{
    ph_collission ret = {0};
    ret.og = bd;
 
    while (i < cl->count && !detect_collission(ret.og, &cl->items[i])) i++;
    if (i < cl->count)
    {
        ret.cl = &cl->items[i];
        ret.i  = i;
    }
    return (ret);
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
