#include "sumo.h"
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 700
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void lines_draw_circle(vec2 p, float r)
{
    vec2 points[32] = {};
    for (u32 i = 0; i < 32; i++)
    {
        float a = TWO_PI * i / 31.0f;
        vec2 d = vec2(cos(a), sin(a)) * r;
        points[i] = p + d;
    }
    lines_draw_poly(points, 32);
}

void init()
{
    lines_init();
}

void tick(Input io, float t, float dt)
{
    clearc(0.0f, 0.01f, 0.02f, 1.0f);

    #if 0
    vec2 a = vec2(-5.0f + 10.0f*io.mouse.pos.x/WINDOW_WIDTH,
                  +5.0f - 10.0f*io.mouse.pos.y/WINDOW_HEIGHT);
    vec2 b = vec2(2, 1);
    vec2 r = normalize(vec2(1, 1));
    vec2 q = normalize(vec2(-0.2, -1));
    float R = (a.x*q.y - a.y*q.x - b.x*q.y + b.y*q.x) / (1.0f - dot(q,r));
    float T = (dot(a, q-r) - dot(b, q-r)) / (dot(q, r) - 1.0f);

    vec2 p = b - q*T;
    vec2 c = a + vec2(r.y, -r.x)*R;

    lines_set_scale(1.0f / 5.0f);
    // lines_draw_line();
    lines_draw_circle(a, 0.1f);
    lines_draw_circle(b, 0.1f);
    lines_draw_circle(p, 0.1f);
    lines_draw_circle(c, R);
    lines_draw_line(a, a + r);
    lines_draw_line(p, p + q);
    lines_draw_line(b, b + q);

    #elif 0

    float R = 1.0f;
    lines_set_scale(1.0f / 5.0f);
    vec2 p1 = vec2(-1.0f, -1.0f);
    vec2 p2 = vec2(0.0f, 3.0f);
    vec2 p3 = vec2(3.0f, 3.0f*sin(t));
    lines_draw_line(p1, p2);
    lines_draw_line(p2, p3);

    vec2 r12 = normalize(p2 - p1);
    vec2 r23 = normalize(p3 - p2);
    vec2 r12_perp = vec2(-r12.y, r12.x);
    vec2 r23_perp = vec2(-r23.y, r23.x);
    float det = r12.x*r23.y - r12.y*r23.x;
    float sign = -det / abs(det);

    float pt = (r23_perp.x - r12_perp.x) * sign * R / (r23.x + r12.x);
    vec2 p12 = p2 - r12 * pt;
    vec2 p23 = p2 + r23 * pt;

    vec2 c = p2 - r12 * pt - r12_perp * sign * R;

    vec4 red = vec4(1.0f, 0.3f, 0.1f, 1.0f);
    vec4 green = vec4(0.3f, 1.0f, 0.1f, 1.0f);
    vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    lines_set_width(2.0f);
    lines_set_color(white);
    lines_draw_circle(p1, 0.1f);
    lines_draw_circle(p2, 0.1f);
    lines_draw_circle(p3, 0.1f);
    lines_draw_circle(p12, 0.1f);
    lines_draw_circle(p23, 0.1f);

    lines_set_color(red);   lines_draw_line(p12, p12 + r12);
    lines_set_color(green); lines_draw_line(p12, p12 + r12_perp);

    lines_set_color(red);   lines_draw_line(p23, p23 + r23);
    lines_set_color(green); lines_draw_line(p23, p23 + r23_perp);

    lines_set_color(white);
    lines_draw_circle(c, R);

    #elif 0

    #define Path_Nodes 5
    float R[Path_Nodes] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };
    vec2 wp[Path_Nodes] = {
        vec2(-600.0f, 0.0f),
        vec2(4680.0f, 3880.0f),
        vec2(7662.0f, 2248.0f),
        vec2(10644.0f, 3878.0f),
        vec2(13626.0f, 2246.0f)
    };

    lines_set_scale(1.0f / 15000.0f);
    lines_set_width(2.0f);
    lines_set_color(vec4(0.3f, 0.3f, 0.3f, 1.0f));
    lines_draw_poly(wp, Path_Nodes);

    lines_set_color(vec4(1.0f, 0.3f, 0.1f, 1.0f));
    for (u32 i = 0; i < Path_Nodes; i++)
        lines_draw_circle(wp[i], 100.0f);

    lines_set_color(vec4(0.3f, 0.7f, 0.5f, 1.0f));
    for (u32 i = 0; i < Path_Nodes-2; i++)
    {
        vec2 p1 = wp[i];
        vec2 p2 = wp[i+1];
        vec2 p3 = wp[i+2];

        vec2 r12 = normalize(p2 - p1);
        vec2 r23 = normalize(p3 - p2);
        vec2 r12_perp = vec2(-r12.y, r12.x);
        vec2 r23_perp = vec2(-r23.y, r23.x);
        float det = r12.x*r23.y - r12.y*r23.x;
        float sign = -det / abs(det);

        float pt = (r23_perp.x - r12_perp.x) * sign * R[i] / (r23.x + r12.x);
        vec2 p12 = p2 - r12 * pt;
        vec2 p23 = p2 + r23 * pt;

        vec2 c = p2 - r12 * pt - r12_perp * sign * R[i];
        lines_draw_circle(c, R[i]);
    }

    #elif 0

    // Hermitian

    #define Path_Nodes 5
    float R[Path_Nodes] = { 500.0f, 500.0f, 500.0f, 500.0f, 500.0f };
    vec2 wp[Path_Nodes] = {
        vec2(sin(t)*5000.0f, 8000.0f),
        vec2(680.0f, -2000.0f),
        vec2(7662.0f, 8248.0f),
        vec2(10644.0f, 3878.0f),
        vec2(13626.0f, 2246.0f)
    };

    lines_set_scale(1.0f / 15000.0f);
    lines_set_width(2.0f);
    lines_set_color(vec4(0.3f, 0.3f, 0.3f, 1.0f));
    lines_draw_poly(wp, Path_Nodes);

    lines_set_color(vec4(1.0f, 0.3f, 0.1f, 1.0f));
    for (u32 i = 0; i < Path_Nodes; i++)
        lines_draw_circle(wp[i], 100.0f);

    lines_set_color(vec4(0.3f, 0.7f, 0.5f, 1.0f));
    for (u32 i = 0; i < Path_Nodes-2; i++)
    {
        vec2 p1 = wp[i];
        vec2 p2 = wp[i+1];
        vec2 p3 = wp[i+2];

        vec2 r12 = normalize(p2 - p1);
        vec2 r23 = normalize(p3 - p2);
        vec2 r12_perp = vec2(-r12.y, r12.x);
        vec2 r23_perp = vec2(-r23.y, r23.x);
        float det = r12.x*r23.y - r12.y*r23.x;
        float sign = -det / abs(det);

        float pt = (r23_perp.x - r12_perp.x) * sign * R[i] / (r23.x + r12.x);
        vec2 p12 = p2 - r12 * pt;
        vec2 p23 = p2 + r23 * pt;

        vec2 c = p2 - r12 * pt - r12_perp * sign * R[i];
        lines_draw_circle(c, R[i]);
    }

    #else

    persist vec2 pd = vec2(5.0f, 5.0f);
    persist vec2 p = vec2(0.0f, 0.0f);
    persist float course = 0.0f;
    persist float speed = 0.0f;

    lines_set_scale(1.0f / 10.0f);
    lines_set_width(2.0f);
    lines_set_color(vec4(1.0f, 0.5f, 0.1f, 1.0f));

    lines_draw_circle(pd, 0.1f);

    vec2 d = pd - p;
    float l = length(d);

    #endif

    lines_flush();
}

#include "sumo.cpp"
