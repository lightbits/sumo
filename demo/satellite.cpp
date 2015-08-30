#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define MULTISAMPLES 4

// TODO: Quaternion attitude control

RenderPass pass;
Mesh cube;

struct Satellite
{
    float m;
    float r;
    vec3 q; // euler angles
    vec3 w; // angular velocity
};

Satellite sat;

void init()
{
    RenderPassSource source = {
        "./shaders/cube.vs",
        "./shaders/cube.fs"
    };
    make_render_pass(&pass, source);
    cube = make_cube();

    sat.m = 80.0f;
    sat.r = 1.2f;
    sat.q = vec3(1.0f, 1.0f, 1.0f);
    sat.w = vec3(0.0f, 0.0f, 0.0f);
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -5.0f) * mat_rotate_x(0.3f) * mat_rotate_y(0.2f);
    mat4 model = mat_scale(0.5f) * mat_rotate_z(sat.q.z) * mat_rotate_y(sat.q.y) * mat_rotate_x(sat.q.x);

    vec3 e1 = vec3(1.0f, sin(sat.q.x) * tan(sat.q.y), cos(sat.q.x) * tan(sat.q.y));
    vec3 e2 = vec3(0.0f, cos(sat.q.x), -sin(sat.q.x));
    vec3 e3 = vec3(0.0f, sin(sat.q.x) / cos(sat.q.y), cos(sat.q.x) / cos(sat.q.y));
    r32 M = sat.m*sat.r*sat.r;
    r32 kd = 2*PI*sat.m*sat.r*sat.r;
    r32 kp = kd;
    vec3 qr = vec3(0.0f, 0.0f, 0.0f);

    r32 h = dt / 50.0f;
    for (u32 i = 0; i < 50; i++)
    {
        sat.w.x += h * (-kd * dot(e1, sat.w) + kp * (qr.x - sat.q.x)) / M;
        sat.w.y += h * (-kd * dot(e2, sat.w) + kp * (qr.y - sat.q.y)) / M;
        sat.w.z += h * (-kd * dot(e3, sat.w) + kp * (qr.z - sat.q.z)) / M;
        sat.q.x += h * dot(e1, sat.w);
        sat.q.y += h * dot(e2, sat.w);
        sat.q.z += h * dot(e3, sat.w);
    }

    begin(&pass);
    depth_test(true, GL_LEQUAL);
    depth_write(true);
    clear(0.35f, 0.55f, 1.0f, 1.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    attribfv("normal", 3, 6, 3);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
}

#include "sumo.cpp"
