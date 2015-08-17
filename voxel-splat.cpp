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

const u32 N = 4;
float sdf[(N+1)*(N+1)*(N+1)];
GLuint splat;
RenderPass pass1;

float eval(float x, float y, float z)
{
    vec3 p = vec3(x, y, z);
    return length(p);
}

GLuint make_splat()
{
    float h = 1.0f / N;
    float v[] = {
        -h, -h, 0.0f, 0.0f, 0.0f,
        +h, -h, 0.0f, 1.0f, 0.0f,
        +h, +h, 0.0f, 1.0f, 1.0f,
        +h, +h, 0.0f, 1.0f, 1.0f,
        -h, +h, 0.0f, 0.0f, 1.0f,
        -h, -h, 0.0f, 0.0f, 0.0f
    };
    return make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

void init()
{
    RenderPassSource source1 = {
        "shaders/splat.vs",
        "shaders/splat.fs"
    };

    pass1 = make_render_pass(source1);

    for (u32 zi = 0; zi <= N; zi++)
    for (u32 yi = 0; yi <= N; yi++)
    for (u32 xi = 0; xi <= N; xi++)
    {
        float x = -1.0f + 2.0f * xi / N;
        float y = -1.0f + 2.0f * yi / N;
        float z = -1.0f + 2.0f * zi / N;
        sdf[zi*N*N + yi*N + xi] = eval(x, y, z);
    }

    splat = make_splat();
}

void tick(float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.2f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -4.0f) *
                mat_rotate_x(t);

    begin(&pass1);
    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    clear(0.35f, 0.55f, 1.0f, 1.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, splat);
    uniformi("N", N);
    uniformf("projection", projection);
    uniformf("view", view);
    attribfv("position", 3, 5, 0);
    attribfv("texel", 2, 5, 3);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, N*N*N);
}

#include "prototype.cpp"
