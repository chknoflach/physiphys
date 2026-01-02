#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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
    physics_body    **items;
    size_t          count;
    size_t          capacity;
} physics_collection;

typedef struct {
    float   g;
    size_t  g_px;
    size_t  fps;
    float   dt;
    float   dt_acc;
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

physics_body    *create_physics_body(void);
void            add_to_collection(physics_collection *, physics_body *);
void            update_physics(gfx_settings *, physics_settings *,
                    physics_collection *, float);

void            _reset(gfx_settings *gfx, physics_collection *c)
{
    physics_body    *bd;
    size_t          count = 7;

    if (c->count != count)
    {
        // TODO: free old memory and stuff
        for (size_t i = 0; i < count; i++)
        {
            bd = create_physics_body();
            add_to_collection(c, bd);
        }
    }
    for (size_t i = 0; i < count; i++)
    {
        bd = c->items[i];
        *bd = DEFAULT_BODY;
        bd->mass = 10.0f * (i + 1);
        bd->drag = 0.2f / (i + 1);
        bd->pos.x = (gfx->width / count) * (i + 1) - (gfx->width / count) / 2;
        bd->dim.y = (i + 1);
        bd->dim.x = (i + 1);
    }
}

int main(void)
{
    gfx_settings gfx = {0};
    physics_settings phys = {0};
    physics_collection bodies = {0};
    float dt;
    physics_body *bd;
    
    gfx.width = 800;
    gfx.height = 600;
    gfx.fps = 60;

    phys.g = 9.81f;
    phys.g_px = phys.g * 200.0f;
    phys.fps = 240;
    phys.dt = 1.0f / phys.fps;
    
    _reset(&gfx, &bodies);

    InitWindow(gfx.width, gfx.height, "raylib test");
    SetTargetFPS(gfx.fps);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();

        if (IsKeyPressed(KEY_R))
            _reset(&gfx, &bodies);
        update_physics(&gfx, &phys, &bodies, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        for (size_t i = 0; i < bodies.count; i++)
        {
            bd = bodies.items[i];
            DrawRectangleLines(bd->pos.x, bd->pos.y, bd->dim.x, bd->dim.y, RED);
            DrawPixel(bd->pos.x, bd->pos.y, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();
    return (0);
}

void    update_physics(gfx_settings *g, physics_settings *s,
        physics_collection *c, float dt)
{
    physics_body *bd;
    s->dt_acc += dt;

    while (s->dt_acc >= s->dt)
    {
        for (size_t i = 0; i < c->count; i++)
        {
            bd = c->items[i];
            float acc_y = (s->g_px * bd->g_tweak)
                - (bd->drag / bd->mass)
                    * bd->v.y * fabsf(bd->v.y);
    
            bd->v.y += acc_y * s->dt;
            bd->pos.y += bd->v.y * s->dt;

            if (bd->pos.y >= (g->height - bd->dim.y))
            {
                bd->v.y = 0;
                bd->pos.y = g->height - bd->dim.y;
            }
        }

        s->dt_acc -= s->dt;
    }
}

void    add_to_collection(physics_collection *cl, physics_body *bd)
{
    if (!cl->capacity || cl->count >= cl->capacity)
    {
        cl->items = realloc(cl->items, sizeof(*bd) * (cl->capacity + MEM_STEP));
        cl->capacity += MEM_STEP;
    }
    cl->items[cl->count] = bd;
    cl->count++;
}

physics_body    *create_physics_body()
{
    physics_body    *body;

    body = malloc(sizeof(*body));
    *body = DEFAULT_BODY;
    return (body);
}
