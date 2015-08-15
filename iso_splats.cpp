/*
https://cg.tuwien.ac.at/~zsolnai/gfx/volumetric-path-tracing-with-equiangular-sampling-in-2k/

http://www.iquilezles.org/www/articles/volumesort/volumesort.htm

Alex Evans - Dreams
*/

#include "prototype.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{

}

void tick(float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    ImGui::NewFrame();
    persist float lightColor[4];
    persist float attenuation;
    ImGui::Begin("Diffuse Shader");
    ImGui::ColorEdit4("lightColor", lightColor);
    ImGui::SliderFloat("attenuation", &attenuation, 1.0f, 16.0f);
    ImGui::End();
    ImGui::Render();
}

#include "prototype.cpp"
