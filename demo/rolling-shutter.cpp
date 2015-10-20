#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *SHADER_VS = GLSL150(
in vec2 position;
out vec2 v_position;
void main()
{
    v_position = position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *SHADER_FS = GLSL150(
in vec2 v_position;
uniform float time;
uniform float deltatime;
uniform float aspect;
uniform float frequency;
out vec4 f_color;

float sphere(vec3 p, float r)
{
    return length(p) - r;
}

vec3 rotate_z(vec3 p, float x)
{
    float c = cos(x);
    float s = sin(x);
    vec3 result;
    result.x = c*p.x - s*p.y;
    result.y = s*p.x + c*p.y;
    result.z = p.z;
    return result;
}

float propeller(vec3 p, float t)
{
    vec3 q = rotate_z(p, frequency*t);
    q.y *= 4.0;
    q.x += 0.9;
    return sphere(q, 1.0);
}

float map(vec3 p, float t)
{
    float p1 = propeller(p, t);
    float p2 = propeller(p, t + 0.7*3.1415/frequency);
    float p3 = propeller(p, t + 1.4*3.1415/frequency);
    return min(min(p1, p2), p3);
}

bool trace_map(float pixeltime, vec3 ro, vec3 rd, out vec3 p, out int i)
{
    float t = 0.0;
    p = ro;
    i = 0;
    while (i < 16)
    {
        p = ro + rd * t;
        float d = map(p, pixeltime);
        if (d < 0.01)
            return true;
        t += d;
        i++;
    }
    return false;
}

vec4 compute(vec2 uv)
{
    vec3 ro = vec3(0.0, 0.2, 4.0);
    vec3 rd = normalize(vec3(uv, -1.0));
    vec3 p;
    int i;
    float pixeltime = time + uv.y * deltatime;
    if (trace_map(pixeltime, ro, rd, p, i))
    {
        return vec4(0.4 + i / 16.0) * vec4(1.0, 0.4, 0.2, 1.0);
    }
    else
    {
        return vec4(1.0);
    }
}

void main()
{
    vec2 uv = v_position * vec2(aspect, 1.0);
    f_color = compute(uv);
}
);

RenderPass pass;
GLuint quad;

void init()
{
    pass = make_render_pass(SHADER_VS, SHADER_FS);
    quad = make_quad();
}

#include <stdio.h>
void tick(Input io, float t, float dt)
{
    persist float speed = 1.0f;
    persist float shutter_hz = 24.0f;
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformf("aspect", (float)WINDOW_WIDTH / WINDOW_HEIGHT);
    uniformf("time", t/speed);
    uniformf("deltatime", 1.0f / shutter_hz);
    uniformf("frequency", speed);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::NewFrame();
    ImGui::Begin("Diffuse Shader");
    ImGui::SliderFloat("shutter [Hz]", &shutter_hz, 1.0f, 60.0f);
    ImGui::SliderFloat("speed", &speed, 1.0f, 64.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
