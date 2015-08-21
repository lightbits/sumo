#include "prototype.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4

RenderPass pass;
Mesh cube;

void init()
{
    RenderPassSource source = {
        "./shaders/cube.vs",
        "./shaders/cube.fs"
    };
    pass = make_render_pass(source);
    cube = make_cube();
}

void tick(float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -5.0f) * mat_rotate_x(0.3f) * mat_rotate_y(0.2f * t);
    mat4 model = mat_scale(1.0f);

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

#include "prototype.cpp"
