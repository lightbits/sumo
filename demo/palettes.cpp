#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{
    lines_init();
    lines_set_width(4.0f);
}

void tick(Input io, float t, float dt)
{
    static float A1 = 0.5f;
    static float A2 = 0.5f;
    static float A3 = 0.5f;
    static float B1 = 0.5f;
    static float B2 = 0.5f;
    static float B3 = 0.7f;
    static float C1 = 0.5f;
    static float C2 = 0.5f;
    static float C3 = 0.5f;
    static float D1 = 0.7f;
    static float D2 = 0.8f;
    static float D3 = 0.8f;

    clearc(0.0f, 0.0f, 0.0f, 1.0f);

    for (int i = 0; i < 256; i++)
    {
        float a = i / 256.0f;
        float x = -1.0f + 2.0f * a;
        float r = A1 + B1 * sin(TWO_PI * (C1 * a + D1));
        float g = A2 + B2 * sin(TWO_PI * (C2 * a + D2));
        float b = A3 + B3 * sin(TWO_PI * (C3 * a + D3));
        lines_set_color(m_vec4(r, g, b, 1.0f));
        lines_draw_line(x, -1.0f, x, +1.0f);
    }

    lines_flush();

    ImGui::NewFrame();
    ImGui::Begin("Diffuse Shader");
    ImGui::SliderFloat("A1", &A1, 0.0f, 1.0f);
    ImGui::SliderFloat("A2", &A2, 0.0f, 1.0f);
    ImGui::SliderFloat("A3", &A3, 0.0f, 1.0f);
    ImGui::SliderFloat("B1", &B1, 0.0f, 1.0f);
    ImGui::SliderFloat("B2", &B2, 0.0f, 1.0f);
    ImGui::SliderFloat("B3", &B3, 0.0f, 1.0f);
    ImGui::SliderFloat("C1", &C1, 0.0f, 1.0f);
    ImGui::SliderFloat("C2", &C2, 0.0f, 1.0f);
    ImGui::SliderFloat("C3", &C3, 0.0f, 1.0f);
    ImGui::SliderFloat("D1", &D1, 0.0f, 1.0f);
    ImGui::SliderFloat("D2", &D2, 0.0f, 1.0f);
    ImGui::SliderFloat("D3", &D3, 0.0f, 1.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
