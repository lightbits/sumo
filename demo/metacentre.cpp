/*
A small heel-angle approximation of the metacentric radius.
*/

#include "sumo.h"
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define MULTISAMPLES 4

void init()
{
    lines_init();
}

void tick(Input io, float t, float dt)
{
    clearc(0, 0, 0, 1);

    float Ly = 1.0f;
    float Lx = 1.0f;
    float Lz = 1.0f;

    float phi = 0.8f*sin(0.5f*t);
    float yb0 = 0.0f;
    float zb0 = -Lz/2.0f;

    float Iyy = 3.0f*Ly*Ly*Ly*Lx / 12.0f; // Area moment of inertia of plane
    float V = 0.5f*Lz*Lx*Ly; // Half of cylinder is submerged

    float yb = yb0 + tan(phi)*Iyy/V;
    float zb = zb0 + 0.5f*tan(phi)*tan(phi)*Iyy/V;

    lines_set_scale(0.5f, 0.5f);

    // Draw water line (y horizontal, z vertical)
    lines_set_color(vec4(0.00f, 0.63f, 0.69f, 1.0f));
    lines_draw_line(-1.8f, 0.0f, +1.8f, 0.0f);

    // Initial buoyancy B
    lines_set_color(vec4(1.0f, 0.5f, 0.2f, 1.0f));
    lines_draw_circle(yb0, zb0, 0.05f);

    // New buoyancy B
    lines_set_color(vec4(0.2f, 1.0f, 0.5f, 1.0f));
    lines_draw_circle(yb, zb, 0.05f);

    // Draw hull
    u32 n = 32;
    float hull_opening = PI/3.0f;
    float hull_radius = Lz;
    for (u32 i = 0; i < n; i++)
    {
        float a0 = PI/2.0f + phi + hull_opening*0.5f + (TWO_PI-hull_opening)*i/(float)n;
        float a1 = PI/2.0f + phi + hull_opening*0.5f + (TWO_PI-hull_opening)*(i+1)/(float)n;
        lines_draw_line(vec2(cos(a0), sin(a0))*hull_radius,
                        vec2(cos(a1), sin(a1))*hull_radius);
    }
    lines_draw_line(0.0f, 0.0f, hull_radius*cos(phi), hull_radius*sin(phi));
    lines_draw_line(0.0f, 0.0f, -hull_radius*cos(phi), -hull_radius*sin(phi));

    lines_set_color(vec4(1.0f, 0.5f, 0.2f, 1.0f));
    lines_draw_line(yb, zb, yb + cos(phi+PI/2.0f)*2.0f, zb + sin(phi+PI/2.0f)*2.0f);
    lines_draw_line(yb0, zb0, yb0, zb0 + 2.0f);

    lines_flush();
}

#include "sumo.cpp"
