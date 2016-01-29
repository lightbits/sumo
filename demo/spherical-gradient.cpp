#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec2 position;
out vec2 v_position;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    v_position = position;
}
);

char *fs = GLSL(
in vec2 v_position;
uniform vec3 forward;
uniform vec3 right;
uniform vec3 up;
uniform vec3 origin;
uniform float aspect;
uniform vec3 direction0;
uniform vec3 direction1;
uniform vec3 direction2;
uniform vec3 direction3;
uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform float exponent0;
uniform float exponent1;
uniform float exponent2;
uniform float exponent3;
uniform float strength0;
uniform float strength1;
uniform float strength2;
uniform float strength3;
out vec4 f_color;
float MAP(vec3 p)
{
    // float d1 = length(p) - 0.5;
    // float d2 = length(p - vec3(0.8, 0.1, 0.0)) - 0.2;
    // float d3 = length(p - vec3(-0.5, 0.1, 0.0)) - 0.4;
    // float d4 = length(p - vec3(0.1, 0.6, 0.0)) - 0.8;
    // return min(min(min(d1, d2), d3), d4);
    return length(p) - 1.0;
}

vec3 NORMAL(vec3 p)
{
    vec2 e = vec2(0.001, 0.0);
    return normalize(vec3(
                     MAP(p + e.xyy) - MAP(p - e.xyy),
                     MAP(p + e.yxy) - MAP(p - e.yxy),
                     MAP(p + e.yyx) - MAP(p - e.yyx)));
}

vec3 SPHERICALGRAD(vec3 normal, vec3 direction, vec3 color, float exponent)
{
    return color * pow(0.5 + 0.5 * dot(direction, normal), exponent);
}

vec3 SHADE(vec3 p)
{
    vec3 color = vec3(0.0, 0.0, 0.0);
    vec3 normal = NORMAL(p);
    color += strength0*SPHERICALGRAD(normal, direction0, color0, exponent0);
    color += strength1*SPHERICALGRAD(normal, direction1, color1, exponent1);
    color += strength2*SPHERICALGRAD(normal, direction2, color2, exponent2);
    color += strength3*SPHERICALGRAD(normal, direction3, color3, exponent3);
    return color;
}

vec3 TRACE(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int step = 0; step < 32; step++)
    {
        float d = MAP(ro + rd * t);
        if (d < 0.01)
        {
            return SHADE(ro + rd * t);
        }
        t += d;
    }
    return vec3(0.0);
}

void main()
{
    vec3 ro = origin;
    vec3 rd = normalize(forward*1.0 + right*v_position.x*aspect + up*v_position.y);
    f_color.rgb = TRACE(ro, rd);
    f_color.rgb = pow(f_color.rgb, vec3(0.4545));
    f_color.a = 1.0;
}
);

RenderPass pass;

void init()
{
    pass = make_render_pass(vs, fs);
}
#include <stdio.h>

void tick(Input io, float t, float dt)
{
    // Camera
    mat3 frame;
    vec3 p;
    {
        persist float theta = 0.0f;
        persist float phi = 0.0f;
        persist float Dtheta = 0.0f;
        persist float Dphi = 0.0f;
        float DDtheta = 0.0f;
        float DDphi = 0.0f;
        if (io.mouse.left.down && !ImGui::GetIO().WantCaptureMouse)
        {
            DDtheta -= dt * 10.0f * io.mouse.rel.x;
            DDphi -= dt * 10.0f * io.mouse.rel.y;
        }
        DDtheta -= 1.0f * Dtheta;
        DDphi -= 1.0f * Dphi;
        Dtheta += DDtheta * dt;
        Dphi += DDphi * dt;
        theta += Dtheta * dt;
        phi += Dphi * dt;
        frame = m_mat3(mat_rotate_y(theta)*mat_rotate_x(phi));
        p = 2.0f*(frame * m_vec3(0.0f, 0.0f, 1.0f));
    }

    persist vec3 direction0 = m_vec3(1.0f, 1.0f, 1.0f);
    persist vec3 direction1 = m_vec3(1.0f, 1.0f, 1.0f);
    persist vec3 direction2 = m_vec3(1.0f, 1.0f, 1.0f);
    persist vec3 direction3 = m_vec3(1.0f, 1.0f, 1.0f);
    persist vec3 color0 = m_vec3(1.0f, 0.4f, 0.4f);
    persist vec3 color1 = m_vec3(1.0f, 0.4f, 0.4f);
    persist vec3 color2 = m_vec3(1.0f, 0.4f, 0.4f);
    persist vec3 color3 = m_vec3(1.0f, 0.4f, 0.4f);
    persist float exponent0 = 8.0f;
    persist float exponent1 = 8.0f;
    persist float exponent2 = 8.0f;
    persist float exponent3 = 8.0f;
    persist float strength0 = 1.0f;
    persist float strength1 = 0.0f;
    persist float strength2 = 0.0f;
    persist float strength3 = 0.0f;

    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    {
        uniformf("forward", -frame.a3);
        uniformf("right", frame.a1);
        uniformf("up", frame.a2);
        uniformf("origin", p);
        uniformf("aspect", WINDOW_WIDTH / (float)WINDOW_HEIGHT);
        uniformf("direction0", m_normalize(direction0));
        uniformf("direction1", m_normalize(direction1));
        uniformf("direction2", m_normalize(direction2));
        uniformf("direction3", m_normalize(direction3));
        uniformf("color0", color0);
        uniformf("color1", color1);
        uniformf("color2", color2);
        uniformf("color3", color3);
        uniformf("exponent0", exponent0);
        uniformf("exponent1", exponent1);
        uniformf("exponent2", exponent2);
        uniformf("exponent3", exponent3);
        uniformf("strength0", strength0);
        uniformf("strength1", strength1);
        uniformf("strength2", strength2);
        uniformf("strength3", strength3);
        so_draw_fullscreen_quad();
    }

    ImGui::NewFrame();
    {
        ImGui::SliderFloat3("direction0", &direction0.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("color0", &color0.x);
        ImGui::SliderFloat("exponent0", &exponent0, 1.0f, 128.0f);
        ImGui::SliderFloat("strength0", &strength0, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::SliderFloat3("direction1", &direction1.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("color1", &color1.x);
        ImGui::SliderFloat("exponent1", &exponent1, 1.0f, 128.0f);
        ImGui::SliderFloat("strength1", &strength1, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::SliderFloat3("direction2", &direction2.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("color2", &color2.x);
        ImGui::SliderFloat("exponent2", &exponent2, 1.0f, 128.0f);
        ImGui::SliderFloat("strength2", &strength2, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::SliderFloat3("direction3", &direction3.x, -1.0f, 1.0f);
        ImGui::ColorEdit3("color3", &color3.x);
        ImGui::SliderFloat("exponent3", &exponent3, 1.0f, 128.0f);
        ImGui::SliderFloat("strength3", &strength3, 0.0f, 1.0f);
    }
    ImGui::Render();
}

#include "sumo.cpp"
