/* https://www.solidangle.com/research/egsr2012_volume.pdf */
#include "prototype.h"
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define MULTISAMPLES 0
#define WINDOW_TITLE "Volumetric Path Tracing with Equiangular Sampling"
#define FPS_LOCK 60

RenderPass pass;
GLuint quad;

GLuint make_quad()
{
    float v[] = {
        -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f,
        +1.0f, +1.0f, -1.0f, +1.0f, -1.0f, -1.0f
    };
    return make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

void init()
{
    RenderPassSource source = {
        "./shaders/equiangular-sampling.vs",
        "./shaders/equiangular-sampling.fs"
    };
    pass = make_render_pass(source);
    quad = make_quad();
}

void tick(float t, float dt)
{
    begin(&pass);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformf("time", t);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    static float dts[256];
    static int dti = 0;
    dts[dti] = dt;
    dti = (dti + 1) % 256;

    ImGui::NewFrame();
    ImGui::Begin("App");
    char text[256]; sprintf(text, "dt: %.2f ms", 1000.0f * dt);
    ImGui::PlotLines("", dts, 256, 0, 0, 0.0f, 0.05f, ImVec2(256, 100));
    ImGui::Text(text);
    ImGui::End();
    ImGui::Render();
}

#include "prototype.cpp"
