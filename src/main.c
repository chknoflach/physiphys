#include "physiphys.h"

void game_init(game_state *st, const char *cfg)
{
    st->cfg.gfx.fps = 60;
    st->cfg.gfx.width = 1280;
    st->cfg.gfx.height = 960;
    st->cfg.ph.g = 9.81f;
    st->cfg.ph.hz = 120;

    st->flags.draw |= DRAW_FLAG_LOG;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
}

int main(void)
{
    game_state state = {0};

    game_init(&state, "./path/to/config");
}

//
//    state.flags |= FLAG_LOG;
//
//    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
//    InitWindow(state.gfx.width, state.gfx.height, "raylib test");
//    _reset(&state);
//
//    while (!WindowShouldClose())
//    {
//        state.dt = GetFrameTime();
//        update_inputs(&state);
//        update_game(&state);
//        update_physics(&state);
//        BeginDrawing();
//        update_draw(&state);
//        EndDrawing();
//    }
//    CloseWindow();
//    return (0);

/*
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
        {
            spawn_physics_body(&state->c,
                    create_body_xy(GetMousePosition(), 
                        (Vector3){
                            state->size,
                            state->size,
                            state->size},
                        (Vector3){
                            2000 * (0.5f - ((float)rand() / RAND_MAX)),
                            2000 * (0.5f - ((float)rand() / RAND_MAX)),
                            0}));
        }
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
    DrawText(TextFormat("%ld", state->stats.collisions), 70, 90, 10, LIGHTGRAY);
}

ph_body create_body_xy(Vector2 pos, Vector3 dim, Vector3 v)
{
    ph_body bd;

    bd = create_ph_body(2.0f, ((float)rand() / RAND_MAX), dim.x, dim.y, dim.z);
    update_ph_body_pos(&bd, pos.x - bd.dim.x / 2, pos.y - bd.dim.y / 2, 0);
    update_ph_body_v(&bd, v.x, v.y, v.z);
    return (bd);
}

void spawn_physics_body(ph_collection *c, ph_body bd)
{
    ph_collision   check;

    check = get_collision(c, &bd, 0);
    if (check.cl == 0)
        add_to_collection(c, bd);
}

int detect_collision(ph_body *a, ph_body *b)
{
    if (a == b)
        return (0);
    if (a->pos.x + a->dim.x <= b->pos.x || b->pos.x + b->dim.x <= a->pos.x)
        return (0);
    if (a->pos.y + a->dim.y <= b->pos.y || b->pos.y + b->dim.y <= a->pos.y)
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

            if (bd->settled || bd->stationary)
                continue;

            state->stats.active++;

            Vector2 acc = {0}; 
            int     ix = (int)floorf(bd->pos.x);
            int     iw = (int)ceilf(bd->dim.x);

            if (ix < 0) {
                iw += ix;
                ix = 0;
            }
            if (iw + ix >= state->gfx.width)
                iw = state->gfx.width - ix;

            for (int k = 0; k < COLLISSION_PASSES; k++)
            {
                if (!resolve_collision(get_collision(&state->c, bd, 0)))
                    break;
                state->stats.collisions++;
                if (bd->stationary || bd->settled)
                    break;
            }

            if (bd->stationary || bd->settled)
                continue;

            acc.y = (state->ph.g_px * bd->g_tweak) -
                (bd->drag / bd->mass) * bd->v.y * fabsf(bd->v.y);
            
            acc.x = 0.0f -
                (bd->drag / bd->mass) * bd->v.x * fabsf(bd->v.x);
            
            update_ph_body_v(bd,
                bd->v.x + acc.x * 1.0f / state->ph.fps,
                bd->v.y + acc.y * 1.0f / state->ph.fps,
                bd->v.z);
            
            update_ph_body_pos(bd,
                bd->pos.x + bd->v.x * 1.0f / state->ph.fps,
                bd->pos.y + bd->v.y * 1.0f / state->ph.fps,
                bd->pos.z);
        }
        state->dt_phys -= 1.0 / state->ph.fps;
    }
}

void    update_ph_body_v(ph_body *bd, float x, float y, float z)
{
    bd->v_prev = bd->v;
    bd->v.x = x;
    bd->v.y = y;
    bd->v.z = z;
}

void    update_ph_body_pos(ph_body *bd, float x, float y, float z)
{
    bd->pos_prev = bd->pos;
    bd->pos.x = x;
    bd->pos.y = y;
    bd->pos.z = z;
}

bool    resolve_collision(ph_collision clash)
{
    ph_body *a = clash.og;
    ph_body *b = clash.cl;
    bool    st = false;

    if (!clash.cl)
        return (false);

    if (clash.og->stationary || clash.og->settled)
    {
        a = clash.og;
        b = clash.cl;
        st = true;
    }
    if (clash.cl->stationary || clash.cl->settled)
    {
        a = clash.cl;
        b = clash.og;
        st = true;
    }
    if (st)
    {
        if (b->pos_prev.y + b->dim.y <= a->pos.y)
            update_ph_body_pos(b, b->pos.x, a->pos.y - b->dim.y, b->pos.z);
        else if (b->pos_prev.y >= a->pos.y + a->dim.y)
            update_ph_body_pos(b, b->pos.x, a->pos.y + a->dim.y, b->pos.z);
        update_ph_body_v(b, b->v.x, 0.0f, b->v.z);
        b->settled = true;
    }
    // next whole if-scope is GPT, the math deep dive is not for me
    if (!st)
    {
        float overlap_x =
            fminf(a->pos.x + a->dim.x, b->pos.x + b->dim.x)
          - fmaxf(a->pos.x, b->pos.x);
    
        float overlap_y =
            fminf(a->pos.y + a->dim.y, b->pos.y + b->dim.y)
          - fmaxf(a->pos.y, b->pos.y);
    
        float invA = 1.0f / a->mass;
        float invB = 1.0f / b->mass;
        float invSum = invA + invB;
    
        const float slop = 0.001f;
    
        Vector2 n = {0};
        float   pen;
    
        if (overlap_x < overlap_y)
        {
            n.x = (a->pos.x + a->dim.x * 0.5f < b->pos.x + b->dim.x * 0.5f) ? -1.0f : 1.0f;
            pen = overlap_x - slop;
        }
        else
        {
            n.y = (a->pos.y + a->dim.y * 0.5f < b->pos.y + b->dim.y * 0.5f) ? -1.0f : 1.0f;
            pen = overlap_y - slop;
        }
    
        // positional correction (split by inverse mass)
        if (pen > 0.0f)
        {
            float corrA = pen * (invA / invSum);
            float corrB = pen * (invB / invSum);
            a->pos.x += n.x * corrA;
            a->pos.y += n.y * corrA;
            b->pos.x -= n.x * corrB;
            b->pos.y -= n.y * corrB;
        }
    
        // impulse along normal (e = 0, no friction)
        float rvx = b->v.x - a->v.x;
        float rvy = b->v.y - a->v.y;
        float vn  = rvx * (-n.x) + rvy * (-n.y);   // relative speed into the normal
    
        if (vn < 0.0f)
        {
            float j = -vn / invSum;                // e=0
            a->v.x -= (-n.x) * (j * invA);
            a->v.y -= (-n.y) * (j * invA);
            b->v.x += (-n.x) * (j * invB);
            b->v.y += (-n.y) * (j * invB);
        }
    }
    return (true);
}

ph_collision get_collision(ph_collection *c, ph_body *bd, size_t i)
{
    ph_collision ret = {0};
    ret.og = bd;
 
    while (i < c->count && !detect_collision(ret.og, &c->items[i])) i++;
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
*/
