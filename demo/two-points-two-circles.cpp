#include "sumo.h"
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{
    lines_init();
}

void tick(Input io, float t, float dt)
{
    persist float x1 = -0.5f;
    persist float y1 = -0.5f;
    persist float x2 = 0.5f;
    persist float y2 = 0.5f;
    persist float r = 0.8f;

    float xm = (x1 + x2) / 2.0f;
    float ym = (y1 + y2) / 2.0f;
    float dx = x1 - x2;
    float dy = y1 - y2;
    float d2 = dx*dx+dy*dy;
    float d = sqrt(d2);
    float px = -dy / d;
    float py = dx / d;
    float h = sqrt(r*r-d2/4.0f);

    float cx1 = xm + px * h;
    float cy1 = ym + py * h;
    float cx2 = xm - px * h;
    float cy2 = ym - py * h;

    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    lines_set_scale(1.0f, 1.0f);
    lines_set_color(0xff3300ff);
    lines_draw_line(x1, y1, x2, y2);
    lines_draw_circle(cx1, cy1, r, 64);
    lines_draw_circle(cx2, cy2, r, 64);
    lines_flush();

    ImGui::NewFrame();
    ImGui::SliderFloat("x1", &x1, -1.0f, +1.0f);
    ImGui::SliderFloat("x2", &x2, -1.0f, +1.0f);
    ImGui::SliderFloat("y1", &y1, -1.0f, +1.0f);
    ImGui::SliderFloat("y2", &y2, -1.0f, +1.0f);
    ImGui::SliderFloat("r", &r, 0.1f, 1.0f);
    ImGui::Render();
}

#include "sumo.cpp"
