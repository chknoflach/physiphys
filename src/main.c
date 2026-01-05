#include "physiphys.h"

/*
 * Next Steps:
 *
 * - Positioning & size relative to window dimensions
 * - World Boundaries instead of Window (incl. coordinate system)
 * - Stability & falling
 * - x-velocity
 * - material bouncy-ness
 * - drag rework: air drag vs. surface-friction
 * - object rotation
 * - stationary element placement
 * - 4-directional element resizing
 * - element preview
 * - different shapes (circle, triangle)
 * 
 * Optimization:
 * - physics grid
 * - multithreading
 */

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

int main(void)
{
    game_state  state = {0};

    state.ups = 720;
    state.size = 5;
    
    state.gfx.width = 800;
    state.gfx.height = 600;
    state.gfx.fps = 60;

    state.ph.g = 9.81f;
    state.ph.g_px = state.ph.g * 200.0f;
    state.ph.fps = 240;

    state.flags |= FLAG_LOG;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(state.gfx.width, state.gfx.height, "raylib test");
    _reset(&state);

    while (!WindowShouldClose())
    {
        state.dt = GetFrameTime();
        update_inputs(&state);
        update_game(&state);
        update_physics(&state);
        BeginDrawing();
        update_draw(&state);
        EndDrawing();
    }
    CloseWindow();
    return (0);
}

void    update_inputs(game_state *state)
{
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R))
        state->defer |= DEFER_RESET;
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_L))
        state->flags = state->flags ^ FLAG_LOG;
    if (IsKeyDown(KEY_UP))
        state->defer |= DEFER_SIZE_PLUS;
    if (IsKeyDown(KEY_DOWN) && state->size >= 0.01f)
        state->defer |= DEFER_SIZE_MINUS;
    if (IsWindowResized())
    {
        state->gfx.width = GetScreenWidth();
        state->gfx.height = GetScreenHeight();
    }
}

void    update_game(game_state *state)
{
    state->dt_game += state->dt;
    if (state->dt_game >= 1.0 / state->ups) {
        if (state->defer & DEFER_RESET)
        {
            _reset(state);
            state->defer ^= DEFER_RESET;
        }
        if (state->defer & DEFER_SIZE_PLUS)
        {
            state->size += 0.01f;
            state->defer ^= DEFER_SIZE_PLUS;
        }
        if (state->defer & DEFER_SIZE_MINUS)
        {
            state->size -= 0.01f;
            state->defer ^= DEFER_SIZE_MINUS;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            spawn_physics_body(&state->c,
                    create_body_xy(GetMousePosition(), 
                        (Vector3){state->size, state->size, state->size}));
        state->dt_game -= 1.0 / state->ups;
    }
}

void    update_draw(game_state *state)
{
    state->dt_draw += state->dt;
    if (state->dt_draw >= 1.0 / state->gfx.fps) {
        ClearBackground(BLACK);
        draw_collection(&state->c);
        if (state->flags & FLAG_LOG)
            draw_log(state);
        state->dt_draw = 0;
    }
}

void    draw_log(game_state *state)
{
    float effps = GetFPS();
    Color fpsc = GREEN;
    if (effps / state->gfx.fps < 0.9)
        fpsc = YELLOW;
    if (effps / state->gfx.fps < 0.6)
        fpsc = ORANGE;
    if (effps / state->gfx.fps < 0.4)
        fpsc = RED;
    DrawText(TextFormat("FPS: %d", (int)(effps)), 10, 10, 10, fpsc);
    DrawText(TextFormat("Phys Entities:"), 10, 30, 10, LIGHTGRAY);
    DrawText(TextFormat("Total:"), 10, 45, 10, LIGHTGRAY);
    DrawText(TextFormat("Active:"), 10, 60, 10, LIGHTGRAY);
    DrawText(TextFormat("Dropped:"), 10, 75, 10, LIGHTGRAY);
    DrawText(TextFormat("Colls:"), 10, 90, 10, LIGHTGRAY);
    DrawText(TextFormat("%ld", state->c.count), 70, 45, 10, LIGHTGRAY);
    DrawText(TextFormat("%ld", state->stats.active), 70, 60, 10, LIGHTGRAY);
    DrawText(TextFormat("%ld", state->stats.dropped), 70, 75, 10, LIGHTGRAY);
    DrawText(TextFormat("%ld", state->stats.collissions), 70, 90, 10, LIGHTGRAY);
}

ph_body create_body_xy(Vector2 pos, Vector3 dim)
{
    ph_body bd;

    bd = create_ph_body(2.0f, 0.2f, dim.x, dim.y, dim.z);
    bd.pos.x = pos.x - bd.dim.x / 2;
    bd.pos.y = pos.y - bd.dim.y / 2;
    bd.drag = ((float)rand() / RAND_MAX);
    return (bd);
}

void spawn_physics_body(ph_collection *c, ph_body bd)
{
    ph_collission   check;

    check = get_collission(c, &bd, 0);
    if (check.cl == 0)
        add_to_collection(c, bd);
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

void _reset(game_state *state)
{
    state->c.count = 0;
    state->c.capacity = 0;
    free(state->c.items); 
    state->c.items = NULL;

    ph_body ground = create_ph_body(0.0f, 0.0f, 400.0f, 20.0f, 1.0f);
    ground.pos.x = (state->gfx.width - 400.0f) / 2;
    ground.pos.y = (state->gfx.height - 20.0f);
    ground.stationary = true;
    ground.material.color = SKYBLUE;
    add_to_collection(&state->c, ground);
}

int sort_ph_collection_comp(const void *a, const void *b)
{
    float ca = ((ph_body*)a)->pos.y + ((ph_body*)a)->dim.y;
    float cb = ((ph_body*)b)->pos.y + ((ph_body*)b)->dim.y;

    return ((ca > cb) - (ca < cb));
}

void update_physics(game_state *state)
{
    ph_body *bd;
    int     max_steps = 1024;
    size_t  i;

    state->dt_phys += state->dt;
    while (max_steps-- && (state->dt_phys >= 1.0f / state->ph.fps))
    {
        i = 0;
        state->stats.active = 0;
        while (i < state->c.count)
        {
            bd = &(state->c.items[i]);
            
            // prune elements out of bounds that aren't moving towards bounds
            if (   (bd->pos.y >= (float)state->gfx.height && bd->v.y >= 0)
                || (bd->pos.y <= 0.0f - bd->dim.y && bd->v.y <= 0) 
                || (bd->pos.x >= (float)state->gfx.width && bd->v.x >= 0)
                || (bd->pos.x <= 0.0f - bd->dim.x && bd->v.x <= 0))
            {
                state->stats.dropped++;
                remove_from_collection(&state->c, i);
                continue;
            }
            i++;

            // skip elements at rest
            if (bd->settled || bd->stationary)
                continue;
            state->stats.active++;
            float   acc_y = 0.0f; 
            int     ix = (int)floorf(bd->pos.x);
            int     iw = (int)ceilf(bd->dim.x);

            if (ix < 0) {
                iw += ix;
                ix = 0;
            }
            if (iw + ix >= state->gfx.width)
                iw = state->gfx.width - ix;

            if (resolve_collission(get_collission(&state->c, bd, 0)))
                state->stats.collissions++;

            if (bd->stationary || bd->settled)
                continue;

            acc_y = (state->ph.g_px * bd->g_tweak) -
                (bd->drag / bd->mass) * bd->v.y * fabsf(bd->v.y);
            bd->v.y += acc_y * 1.0f / state->ph.fps;
            bd->pos.y += bd->v.y * 1.0f / state->ph.fps;
        }
        state->dt_phys -= 1.0 / state->ph.fps;
    }
}

bool    resolve_collission(ph_collission clash)
{
    if (!clash.cl)
        return (false);

    ph_body *st = NULL;
    ph_body *mv = NULL;

    if (clash.og->v.y == 0 && (clash.og->stationary || clash.og->settled))
    {
        st = clash.og;
        mv = clash.cl;
    }
    if (clash.cl->v.y == 0 && (clash.cl->stationary || clash.cl->settled))
    {
        st = clash.cl;
        mv = clash.og;
    }
    if (st)
    {
        mv->pos.y = st->pos.y
            - (mv->dim.y * (mv->v.y > 0))
            + (st->dim.y * (mv->v.y < 0));
        mv->v.y = 0.0f;
        mv->settled = true;
        return (true);
    }

    clash.og->pos.y = clash.cl->pos.y > clash.og->pos.y ?
        clash.cl->pos.y - clash.og->dim.y : clash.og->pos.y;
    clash.cl->pos.y  = clash.cl->pos.y < clash.og->pos.y ?
        clash.og->pos.y - clash.cl->dim.y : clash.cl->pos.y;
    float v = (clash.og->mass * clash.og->v.y
                + clash.cl->mass * clash.cl->v.y)
            / (clash.og->mass + clash.cl->mass);
    clash.og->v.y = v;
    clash.cl->v.y = v;
    return (true);
}

ph_collission get_collission(ph_collection *c, ph_body *bd, size_t i)
{
    ph_collission ret = {0};
    ret.og = bd;
 
    while (i < c->count && !detect_collission(ret.og, &c->items[i])) i++;
    if (i < c->count)
    {
        ret.cl = &c->items[i];
        ret.i  = i;
    }
    return (ret);
}

void    add_to_collection(ph_collection *c, ph_body bd)
{
    if (c->count >= c->capacity)
    {
        if (c->capacity < MEM_STEP)
            c->capacity = MEM_STEP;
        else
            c->capacity *= 2;
        c->items = realloc(c->items, sizeof(bd) * c->capacity);
    }
    c->items[c->count] = bd;
    c->count++;
}

void    remove_from_collection(ph_collection *c, size_t index)
{
    if (index >= c->count)
        return ;

    c->count--;
    if (index < c->count)
    {
        // O(1) and preserves order
        memmove(&c->items[index],
                &c->items[index + 1],
                (c->count - index) * sizeof(*c->items));
    }
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
