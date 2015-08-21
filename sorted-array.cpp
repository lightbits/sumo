#include "prototype.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

RenderPass pass1;
GLuint vbo;

void init()
{
    SortedArray_r32 a = {};
    sorted_array_alloc(&a, 16);
    sorted_array_set(&a, "acb", 4.4f);
    sorted_array_set(&a, "ab", 3.3f);
    sorted_array_set(&a, "aa", 2.2f);
    sorted_array_set(&a, "a", 1.1f);

    for (int i = 0; i < sorted_array_len(&a); i++)
    {
        char *key = a.base.keys[i];
        float *x = sorted_array_get(&a, key, float);
        if (x)
            printf("%d : %s = %f\n", i, key, *x);
    }

    RenderPassSource pass1_src = {
        "shaders/hello.vs",
        "shaders/hello.fs"
    };

    pass1 = make_render_pass(pass1_src);

    float v[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        +1.0f, +1.0f,
        +1.0f, +1.0f,
        -1.0f, +1.0f,
        -1.0f, -1.0f
    };
    vbo = make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

void tick(float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass1);
    uniformf("blue_color", sin(t));
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    attribv("position", GL_FLOAT, 2, 2 * sizeof(GLfloat), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#include "prototype.cpp"