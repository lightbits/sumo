#include "prototype.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4

void init()
{

}

void tick(float t, float dt)
{
    Clearc(1.0f, 0.95f, 0.8f, 1.0f);
    ImGui::NewFrame();
    static float lightColor[4];
    static float attenuation;
    ImGui::Begin("Diffuse Shader");
    ImGui::ColorEdit4("lightColor", lightColor);
    ImGui::SliderFloat("attenuation", &attenuation, 1.0f, 16.0f);
    ImGui::End();
    ImGui::Render();
}

#include "prototype.cpp"
