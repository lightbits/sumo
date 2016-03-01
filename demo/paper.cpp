/*
Curl noise
Scale and rotation along flow
Shadow maps
*/

#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec3 object;
in vec3 normal;
uniform vec3 color;
uniform vec3 offset;
uniform mat3 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 v_color;
out vec3 v_normal;
void main()
{
    v_color = color;
    v_normal = model * normal;
    gl_Position = projection * view * vec4(model * object + offset, 1.0);
}
);

char *fs = GLSL(
in vec3 v_color;
in vec3 v_normal;
out vec4 f_color;
void main()
{
    vec3 l = normalize(vec3(1.0, 1.1, 1.3));
    vec3 n = normalize(v_normal);
    float ndotl = 0.5f + 0.5f * dot(n, l);
    f_color = vec4(v_color, 1.0) * ndotl;
}
);

RenderPass pass;
Mesh cube;

struct Particle
{
    mat3 model;
    vec3 color;
    vec3 offset;
};

Particle particles[2048];

float snoise(float x, float y)
{
    float xi = floor(x);
    float yi = floor(y);
    float xf = x - xi;
    float yf = y - yi;

    float h00 = noise2f(xi, yi);
    float h10 = noise2f(xi + 1, yi);
    float h01 = noise2f(xi, yi + 1);
    float h11 = noise2f(xi + 1, yi + 1);

    float u = m_smoothstep(0.0f, 1.0f, xf);
    float v = m_smoothstep(0.0f, 1.0f, yf);

    float x0 = h00 + (h10 - h00) * u;
    float x1 = h01 + (h11 - h01) * u;

    return x0 + (x1 - x0) * v;
}

float fbm(float x, float y)
{
    float result = 0.0f;
    result += snoise(x, y);
    result += 0.44f * snoise(x * 1.8f, y * 1.8f);
    result += 0.21f * snoise(x * 3.9f, y * 3.9f);
    result += 0.11f * snoise(x * 8.2f, y * 8.2f);
    return result;
}

void init()
{
    u32 colors[] = {
        0xed6a5aff,
        0xf4f1bbff,
        0x9bc1bcff,
        0x5ca4a9ff,
        0xe6ebe0ff,
        0xf0b67fff,
        0xfe5f55ff,
        0xd6d1b1ff,
        0xc7efcfff,
        0xeef5dbff,
        0x50514fff,
        0xf25f5cff,
        0xffe066ff,
        0x247ba0ff,
        0x70c1b3ff
    };

    cube = make_cube();
    pass = make_render_pass(vs, fs);
    for (u32 i = 0; i < array_count(particles); i++)
    {
        particles[i].model = m_id3();
        particles[i].model.a11 = 0.01f + 0.01f * frand();
        particles[i].model.a22 = 0.01f + 0.01f * frand();
        particles[i].model.a33 = 0.01f + 0.01f * frand();
        particles[i].offset = m_vec3(-1.0f + 2.0f * frand(),
                                     -1.0f + 2.0f * frand(),
                                     -1.0f + 2.0f * frand());

        u32 ci = i % array_count(colors);
        particles[i].color.x = (r32)((colors[ci] >> 24) & 0xff) / 255.0f;
        particles[i].color.y = (r32)((colors[ci] >> 16) & 0xff) / 255.0f;
        particles[i].color.z = (r32)((colors[ci] >>  8) & 0xff) / 255.0f;
    }
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -2.0f) * mat_rotate_x(0.3f);

    depth_test(true);
    depth_write(true);
    clear(0.05f, 0.03f, 0.01f, 1.0f, 1.0f);
    begin(&pass);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("object", 3, 6, 0);
    attribfv("normal", 3, 6, 3);
    uniformf("projection", projection);
    uniformf("view", view);

    float dx = 0.001f;
    float dy = 0.001f;
    for (u32 i = 0; i < array_count(particles); i++)
    {
        vec3 p = particles[i].offset;
        p.z = 0.0f;
        float dndx = (fbm(p.x + dx, p.y) - fbm(p.x - dx, p.y)) / dx;
        float dndy = (fbm(p.x, p.y + dy) - fbm(p.x, p.y - dy)) / dy;
        vec3 v = 0.1f * m_vec3(dndy, -dndx, 0.0f);
        vec3 n = m_vec3(v.y, -v.x, 0.0f);
        float speed = m_max(0.01f, m_length(v));
        particles[i].model.a1 = (0.02f + 0.02f*m_smoothstep(0.1f, 0.5f, speed)) * (v / speed);
        particles[i].model.a2 = 0.005f*(n / speed);
        p += v * dt;
        particles[i].offset = p;
    }

    for (u32 i = 0; i < array_count(particles); i++)
    {
        uniformf("color", particles[i].color);
        uniformf("model", particles[i].model);
        uniformf("offset", particles[i].offset);
        glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
    }
}

#include "sumo.cpp"
