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
} physics_body;

typedef struct {
    physics_body    **items;
    int             count;
    int             capacity;
} physics_collection;

typedef struct {
    float   g;
    int     g_px;
    int     fps;
    float   dt;
    float   dt_acc;
} physics_settings;

typedef struct {
    int     width;
    int     height;
    int     fps;
} gfx_settings;

static const physics_body DEFAULT_BODY = {
    .mass = 1.0f,
    .drag = 0.2f,
    .g_tweak = 1.0f,
    .pos.x = 0.0f,
    .pos.y = 0.0f,
    .v.x = 0.0f,
    .v.y = 0.0f
};

physics_body    *create_physics_body(void);
void            add_to_collection(physics_collection *, physics_body *);

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
    phys.g_px = phys.g * 5000.0f;
    phys.fps = 120;
    phys.dt = 1.0f / phys.fps;
    
    bd = create_physics_body();
    bd->mass = 10.0f;
    bd->drag = 0.02f;
    bd->pos.x = gfx.width / 2;
//    add_to_collection(&bodies, bd);

    InitWindow(gfx.width, gfx.height, "raylib test");
    SetTargetFPS(gfx.fps);

    while (!WindowShouldClose())
    {
        dt = GetFrameTime();
        phys.dt_acc += dt;

        while (phys.dt_acc >= phys.dt)
        {
            float acc_y = (phys.g_px * bd->g_tweak)
                - (bd->drag / bd->mass)
                    * bd->v.y * fabsf(bd->v.y);
    
            bd->v.y += acc_y * phys.dt;
            bd->pos.y += bd->v.y * phys.dt;

            if (bd->pos.y > gfx.height)
            {
                bd->v.y = bd->pos.y = 0.0f;
                bd->pos.x += 2.0f;
                if (bd->pos.x > gfx.width)
                    bd->pos.x = 0.0f;
                bd->mass += 1.0f;
            }

            phys.dt_acc -= phys.dt;
        }

        BeginDrawing();
        DrawPixel(bd->pos.x, bd->pos.y, RED);
        EndDrawing();
    }
    CloseWindow();
    return (0);
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
