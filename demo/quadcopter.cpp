/* Quadcopter modelling, simulation and control

* Newton-Euler equations of motion
    - Quaternions or
    - Euler angles
* Disturbances
    - Measurement noise
    - IMU bias and noise model

* Optimal control allocation
    Solve equality-constrained QP for the voltage allocation
    to the torque control inputs.
* Feedback linearization
* Sliding mode control
* Trajectory following
*/

#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

RenderPass pass;
MeshAsset mesh_quad;
GLuint tex_quad;

void init()
{
    pass = load_render_pass("assets/shaders/lowpoly.vs",
                            "assets/shaders/lowpoly.fs");
    mesh_quad = load_mesh("assets/models/quadcopter/quadcopter.sumo_asset");
    tex_quad = so_load_tex2d("assets/models/quadcopter/quadcopter.png", 0, 0,
                             GL_NEAREST, GL_NEAREST);
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = camera_holdclick(io, dt);
    // mat4 view = mat_translate(0.0f, 0.0f, -2.0f) * mat_rotate_x(0.7f) * mat_rotate_y(0.2f);
    mat4 model = mat_scale(0.5f);
    begin(&pass);
    depth_test(true, GL_LEQUAL);
    depth_write(true);
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clear(0.35f, 0.55f, 1.0f, 1.0f, 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_quad);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    uniformi("channel0", 0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_quad.positions);
    attribfv("position", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_quad.normals);
    attribfv("normal", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_quad.texels);
    attribfv("texel", 2, 2, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_quad.indices);
    glDrawElements(GL_TRIANGLES, mesh_quad.num_indices, GL_UNSIGNED_INT, 0);
}

#include "sumo.cpp"
