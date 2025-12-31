#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define FPS 240
#define FPS_PHYS 120

typedef struct {
	float mass;		// mass in kg
	float drag;		// drag coefficient
	float g_tweak;  // manual gravity tweak; 1.0f = normal
	Vector2 v;		// m/s
	Vector2 pos;	// meters
} physics_body;

int main(void)
{
	const int WIDTH = 800;
	const int HEIGHT = 600;
	const float G = 9.81f;
	const int G_PX = G * 5500.0f;
	const float phys_dt = 1.0f / FPS_PHYS;
	float phys_dt_acc = 0.0f;
	float dt;
	physics_body bd;
	
	bd.mass = 10.0f;
	bd.drag = 0.02f;
	bd.g_tweak = 1.0f;
	bd.pos.x = WIDTH / 2;
	bd.pos.y = 0.0f;
	bd.v.x = 0.0f;
	bd.v.y = 0.0f;

    InitWindow(WIDTH, HEIGHT, "raylib test");
	SetTargetFPS(FPS);

	while (!WindowShouldClose())
    {
		dt = GetFrameTime();
		phys_dt_acc += dt;

		while (phys_dt_acc >= phys_dt)
		{
			float acc_y = (G_PX * bd.g_tweak)
				- (bd.drag / bd.mass)
					* bd.v.y * fabsf(bd.v.y);
	
	        bd.v.y += acc_y * phys_dt;
	        bd.pos.y += bd.v.y * phys_dt;

			if (bd.pos.y > HEIGHT)
			{
				bd.v.y = bd.pos.y = 0.0f;
				bd.pos.x += 2.0f;
				if (bd.pos.x > WIDTH)
					bd.pos.x = 0.0f;
				bd.mass += 1.0f;
			}

			phys_dt_acc -= phys_dt;
		}

		BeginDrawing();
		DrawPixel(bd.pos.x, bd.pos.y, RED);
        EndDrawing();
    }
    CloseWindow();
    return (0);
}
