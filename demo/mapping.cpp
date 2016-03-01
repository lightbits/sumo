#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{
    lines_init();
}

void tick(Input io, r32 t, r32 dt)
{
    clearc(0.0f, 0.0f, 0.0f, 1.0f);

    lines_set_scale(0.7f*WINDOW_HEIGHT/(r32)WINDOW_WIDTH, 0.7f);

    r32 t_lim = 30.0f * PI / 180.0f;
    r32 t_min = -t_lim;
    r32 t_max = PI + t_lim;
    r32 lens_radius = 1.2f;

    lines_set_width(2.0f);
    lines_set_color(0x3377ffff);
    for (int ti = 0; ti < 64; ti++)
    {
        r32 t0 = t_min + (t_max-t_min)*ti/64.0f;
        r32 t1 = t_min + (t_max-t_min)*(ti+1)/64.0f;
        r32 x0 = lens_radius*cos(t0);
        r32 z0 = lens_radius*sin(t0);
        r32 x1 = lens_radius*cos(t1);
        r32 z1 = lens_radius*sin(t1);
        lines_draw_line(x0, z0, x1, z1);
    }

    #define num_sample_u 32
    r32 sample_u[num_sample_u];
    lines_set_color(0xff4444ff);
    lines_draw_line(-1.0f, 0.0f, 1.0f, 0.0f);
    for (int ui = 0; ui < num_sample_u; ui++)
    {
        r32 u = -1.0f + 2.0f * ui / (num_sample_u-1);
        sample_u[ui] = u;

        r32 t = t_max + (t_min - t_max) * (0.5f + 0.5f * u);
        r32 x = lens_radius*cos(t);
        r32 z = lens_radius*sin(t);

        lines_draw_line(x, z, x*1.05f, z*1.05f);
        lines_draw_line(u, 0.0f, u, -0.05f);
    }

    lines_flush();
}

#include "sumo.cpp"
