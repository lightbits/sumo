#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

RenderPass pass;
GLuint vbo;

void init()
{
    Map_r32 a = {};
    map_alloc(&a, 16);
    map_set(&a, "acb", 4.4f);
    map_set(&a, "ab", 3.3f);
    map_set(&a, "aa", 2.2f);
    map_set(&a, "a", 1.1f);

    for (int i = 0; i < map_len(&a); i++)
    {
        char *key = a.base.keys[i];
        float *x = map_get(&a, key, float);
        if (x)
            printf("%d : %s = %f\n", i, key, *x);
    }

    float v[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        +1.0f, +1.0f,
        +1.0f, +1.0f,
        -1.0f, +1.0f,
        -1.0f, -1.0f
    };
    vbo = make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    pass = load_render_pass("assets/shaders/hello.vs", "assets/shaders/hello.fs");
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    uniformf("blue_color", sin(t));
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    attribv("position", GL_FLOAT, 2, 2 * sizeof(GLfloat), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#include "sumo.cpp"
