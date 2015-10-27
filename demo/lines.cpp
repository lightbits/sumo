#include "sumo.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#include <stdio.h>

#define NUM_TRACES_X 33
#define NUM_TRACES_Y 33
vec2 points[NUM_TRACES_X * NUM_TRACES_Y];
r32 field_range = 10.0f;

void reset()
{
    clearc(0.0f, 0.0f, 0.0f, 1.0f);

    for (u32 y = 0; y < NUM_TRACES_Y; y++)
    for (u32 x = 0; x < NUM_TRACES_X; x++)
    {
        r32 xf = -1.0f + 2.0f * (x -0.4f + 0.8f*frand()) / (float)(NUM_TRACES_X - 1);
        r32 yf = -1.0f + 2.0f * (y -0.4f + 0.8f*frand()) / (float)(NUM_TRACES_Y - 1);
        points[y * NUM_TRACES_X + x] = vec2(xf, yf) * field_range;
    }
}

void init()
{
    lines_init();
    lines_set_color(V4(0.00f, 0.63f, 0.69f, 0.13f));
    reset();
}

vec2 f(r32 a, r32 b, r32 c, vec2 x)
{
    return vec2(a*x.x - x.x*x.y,
                b*x.x*x.x - c*x.y);
}

void tick(Input io, float t, float dt)
{
    persist r32 a = 3.0f;
    persist r32 b = 0.8f;
    persist r32 c = 0.6f;
    persist r32 sim_speed = 0.05f;
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // for (u32 i = 0; i < NUM_TRACES_X * NUM_TRACES_Y; i++)
    // {
    //     float dx = field_range / NUM_TRACES_X;
    //     float dy = field_range / NUM_TRACES_Y;
    //     lines_set_color(V4(0.95f, 0.35f, 0.1f, 0.02f));
    //     lines_draw_rect(points[i] - V2(dx, dy) * 0.5f, V2(dx, dy));
    //     lines_draw_line(points[i] - V2(dx, dy) * 0.1f,
    //                     points[i] + V2(dx, dy) * 0.1f);
    //     lines_draw_line(points[i] + V2(-dx, dy) * 0.1f,
    //                     points[i] + V2(dx, -dy) * 0.1f);
    // }

    for (u32 j = 0; j < 10; j++)
    for (u32 i = 0; i < NUM_TRACES_X * NUM_TRACES_Y; i++)
    {
        vec2 fi = f(a, b, c, points[i]);
        vec2 q = points[i] + fi * sim_speed * dt;
        float blend = mapf(0.0f, 50.0f, length(fi), 0.0f, 1.0f);
        vec4 color = mixf(V4(0.00f, 0.63f, 0.69f, 0.13f),
                          V4(1.0f, 0.3f, 0.1f, 0.2f),
                          blend);

        lines_set_color(color);
        lines_draw_line(points[i], q);
        points[i] = q;
    }

    field_range += io.mouse.wheel.y * 0.5f;
    if (io.mouse.wheel.y != 0.0f)
        reset();
    lines_set_scale(1.0f / field_range);
    lines_flush();

    ImGui::NewFrame();
    ImGui::Begin("Parameters");

    float x = (-1.0f + 2.0f * io.mouse.pos.x / (float)WINDOW_WIDTH) * field_range;
    float y = (+1.0f - 2.0f * io.mouse.pos.y / (float)WINDOW_HEIGHT) * field_range;

    char location_text[255];
    sprintf(location_text, "x: %.2f, y: %.2f", x, y);

    ImGui::Text(location_text);
    if (ImGui::SliderFloat("a", &a, 0.1f, 10.0f)) reset();
    if (ImGui::SliderFloat("b", &b, 0.1f, 10.0f)) reset();
    if (ImGui::SliderFloat("c", &c, 0.1f, 10.0f)) reset();
    if (ImGui::SliderFloat("speed", &sim_speed, 0.001f, 0.1f)) reset();
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
