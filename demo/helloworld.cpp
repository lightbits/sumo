#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{

}

void tick(Input io, float t, float dt)
{
    ImGui::NewFrame();
    persist float lightColor[4];
    persist float attenuation;
    ImGui::Begin("Diffuse Shader");
    ImGui::ColorEdit4("lightColor", lightColor);
    ImGui::SliderFloat("attenuation", &attenuation, 1.0f, 16.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
