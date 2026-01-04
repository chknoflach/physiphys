#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MEM_STEP 64

typedef struct {
    float   mass;       // mass in kg
    float   drag;       // drag coefficient
    float   g_tweak;    // manual gravity tweak; 1.0f = normal
    Vector2 v;          // m/s
    Vector2 pos;        // meters
    Vector3 dim;        // dimensions as bounding box
} physics_body;

typedef struct {
    physics_body    *items;
    size_t          count;
    size_t          capacity;
} physics_collection;

typedef struct {
    float   g;
    size_t  g_px;
    size_t  fps;
    float   dt;
    float   dt_acc;
    size_t  *col;
} physics_settings;

typedef struct {
    size_t  width;
    size_t  height;
    size_t  fps;
} gfx_settings;

static const physics_body DEFAULT_BODY = {
    .mass = 1.0f,
    .drag = 0.2f,
    .g_tweak = 1.0f,
    .pos.x = 0.0f,
    .pos.y = 0.0f,
    .v.x = 0.0f,
    .v.y = 0.0f,
    .dim.x = 1.0f,
    .dim.y = 1.0f,
    .dim.z = 1.0f
};

physics_body    create_physics_body(float, float, float, float, float);
void            add_to_collection(physics_collection *, physics_body);
void            update_physics(gfx_settings *, physics_settings *,
                    physics_collection *, float);

void _reset(gfx_settings *gfx, physics_settings *phy,
        physics_collection *c)
{
    c->count = 0;
    c->capacity = 0;
    free(c->items);
    c->items = NULL;
    memset(phy->col, 0, gfx->width * sizeof(*phy->col));
}

int main(void)
{
    gfx_settings gfx = {0};
    physics_settings phys = {0};
    physics_collection bodies = {0};
    float dt;
    
    gfx.width = 800;
    gfx.height = 600;
    gfx.fps = 60;

    phys.col = calloc(gfx.width, sizeof(*phys.col));
    phys.g = 9.81f;
    phys.g_px = phys.g * 200.0f;
    phys.fps = 240;
    phys.dt = 1.0f / phys.fps;
    
    InitWindow(gfx.width, gfx.height, "raylib test");
    SetTargetFPS(gfx.fps);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        if (IsKeyPressed(KEY_R))
            _reset(&gfx, &phys, &bodies);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            physics_body    bd = create_physics_body(2.0f, 0.2f, 10.0f, 10.0f, 10.0f);
            bd.pos = GetMousePosition();
            add_to_collection(&bodies, bd);
        }
        update_physics(&gfx, &phys, &bodies, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        for (size_t i = 0; i < bodies.count; i++)
        {
            DrawRectangleLines(bodies.items[i].pos.x, bodies.items[i].pos.y,
                    bodies.items[i].dim.x, bodies.items[i].dim.y,
                    RED);
            DrawPixel(bodies.items[i].pos.x, bodies.items[i].pos.y, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();
    return (0);
}

int sort_physics_collection_comp(const void *a, const void *b)
{
    float ca = ((physics_body*)a)->pos.y + ((physics_body*)a)->dim.y;
    float cb = ((physics_body*)b)->pos.y + ((physics_body*)b)->dim.y;

    return ((ca > cb) - (ca < cb));
}

void update_physics(gfx_settings *gfx, physics_settings *phy,
        physics_collection *c, float dt)
{
    phy->dt_acc += dt;

    while (phy->dt_acc >= phy->dt)
    {
        qsort(c->items, c->count, sizeof(*c->items),
                sort_physics_collection_comp);
        for (size_t i = 0; i < c->count; i++)
        {
            float           acc_y = 0.0f; 
            bool            settled = false;
            size_t          ix = roundf(c->items[i].pos.x);
            size_t          iy = roundf(c->items[i].pos.y);
            size_t          iw = roundf(c->items[i].dim.x);
            size_t          ih = roundf(c->items[i].dim.y);
            size_t          y_max = gfx->height;

            for (size_t j = 0; j < iw; j++)
            {
                if (y_max > gfx->height - phy->col[ix + j])
                    y_max = gfx->height - phy->col[ix + j];
            }

            if ((iy + ih) >= y_max)
            {
                c->items[i].pos.y = y_max - ih;
                c->items[i].v.y = 0.0f;
                for (size_t j = 0; j < iw; j++)
                    phy->col[ix + j] += ih;
                settled = true;
            }

            if (settled)
                continue;
    
            acc_y = (phy->g_px * c->items[i].g_tweak) -
                (c->items[i].drag / c->items[i].mass) * c->items[i].v.y * fabsf(c->items[i].v.y);
            c->items[i].v.y += acc_y * phy->dt;
            c->items[i].pos.y += c->items[i].v.y * phy->dt;
        }

        phy->dt_acc -= phy->dt;
    }
}

void    add_to_collection(physics_collection *cl, physics_body bd)
{
    if (!cl->capacity || cl->count >= cl->capacity)
    {
        cl->items = realloc(cl->items, sizeof(bd) * (cl->capacity + MEM_STEP));
        cl->capacity += MEM_STEP;
    }
    cl->items[cl->count] = bd;
    cl->count++;
}

physics_body    create_physics_body(float mass, float drag, float x, float y, float z)
{
    physics_body    bd;

    bd = DEFAULT_BODY;
    bd.mass = mass;
    bd.drag = drag;
    bd.dim.x = x;
    bd.dim.y = y;
    bd.dim.z = z;
    return (bd);
}

