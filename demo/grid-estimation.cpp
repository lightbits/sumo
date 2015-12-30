#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{
    lines_init();
}

// R: Rotation from view frame to world frame
// p: Camera position from world origin relative world frame
mat4 view_from_se3(mat3 R, vec3 p)
{
    R.a1 *= -1.0f;
    R.a3 *= -1.0f;
    mat3 T = m_transpose(R);
    return m_se3(T, -T*p);
}

// Project a point w in world-space into NDC
vec2 project(mat4 projection, mat4 view, vec3 w)
{
    vec4 clip = projection * view * m_vec4(w, 1.0f);
    if (clip.w > 0.01f)
        return clip.xy / clip.w;
    else
        return clip.xy  / 0.01f;
}

void lines_draw_3d(mat4 projection, mat4 view, vec3 a, vec3 b)
{
    vec2 ap = project(projection, view, a);
    vec2 ab = project(projection, view, b);
    lines_draw_line(ap, ab);
}

#include <stdio.h>
namespace ImGui
{
    void Label(quat q, char *text)
    {
        ImGui::Text("%s: (%.2f %.2f %.2f %.2f)", text, q.x, q.y, q.z, q.w);
    }
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);

    persist vec3 p = m_vec3(1.0f, 1.0f, 2.0f);
    persist quat q = m_vec4(-0.05f, 0.94f, -0.22f, -0.23f);
    mat4 view = view_from_se3(m_quat_to_so3(q), p);

    blend_mode(true);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    s32 n = 32;
    lines_set_color(0x63636377);
    for (s32 i = 0; i < n; i++)
    {
        float a = -1.0f + 2.0f * i / (float)(n-1);
        lines_draw_3d(projection, view, m_vec3(1, 0, a), m_vec3(-1, 0, a));
        lines_draw_3d(projection, view, m_vec3(a, 0, 1), m_vec3(a, 0, -1));
    }
    lines_set_width(2.0f);
    lines_set_color(0xB23F24FF);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(1, 0, 0));
    lines_set_color(0x78B041FF);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(0, 1, 0));
    lines_set_color(0x3490E0FF);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(0, 0, 1));
    lines_flush();

    #if 0
    ImGui::NewFrame();
    ImGui::Begin("Diffuse Shader");
    ImGui::Label(q, "q");
    ImGui::End();
    ImGui::Render();
    #endif
}

#include "sumo.cpp"
