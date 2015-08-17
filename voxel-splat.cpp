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

const u32 N = 32;
GLuint splat;
GLuint sdf;
RenderPass pass1;

float eval(float x, float y, float z)
{
    vec3 p = vec3(x, y, z);
    return length(p) - 0.5;
}

GLuint make_splat()
{
    float v[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        +1.0f, +1.0f,
        +1.0f, +1.0f,
        -1.0f, +1.0f,
        -1.0f, -1.0f
    };
    return make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

GLuint make_sdf()
{
    const u32 N1 = N + 1;
    float f[N1*N1*N1];
    for (u32 zi = 0; zi <= N; zi++)
    for (u32 yi = 0; yi <= N; yi++)
    for (u32 xi = 0; xi <= N; xi++)
    {
        float x = -1.0f + 2.0f * xi / N;
        float y = -1.0f + 2.0f * yi / N;
        float z = -1.0f + 2.0f * zi / N;
        f[zi*N1*N1 + yi*N1 + xi] = eval(x, y, z);
    }
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_3D, result);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED,
                 N1, N1, N1, 0, GL_RED,
                 GL_FLOAT, f);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);
    return result;
}

void init()
{
    RenderPassSource source1 = {
        "shaders/splat.vs",
        "shaders/splat.fs"
    };
    pass1 = make_render_pass(source1);
    sdf = make_sdf();
    splat = make_splat();
}

void tick(float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f,
                      WINDOW_WIDTH, WINDOW_HEIGHT,
                      0.2f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -4.0f) *
                mat_rotate_x(0.3f) *
                mat_rotate_y(t);

    begin(&pass1);
    clear(0.35f, 0.55f, 1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, sdf);

    // glEnable(GL_DEPTH_TEST);
    // glDepthRangef(0.0f, 1.0f);
    // glDepthMask(GL_TRUE);
    // glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, splat);
    uniformi("tex_sdf", 0);
    uniformi("N", N);
    uniformf("projection", projection);
    uniformf("view", view);
    attribfv("quadcoord", 2, 2, 0);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, N*N*N);
}

#include "prototype.cpp"
