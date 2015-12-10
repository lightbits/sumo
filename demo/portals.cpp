#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define WINDOW_HIDE_CURSOR 1
#define GLSL(src) "#version 150\n" #src

char *stencil_vs = GLSL(
in vec3 position;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
});

char *stencil_fs = GLSL(
out vec4 f_color;
void main()
{
    f_color = vec4(1.0);
}
);

struct Renders
{
    RenderPass diffuse;
    RenderPass stencil;
} renders;
MeshAsset world;
GLuint diffuse;
Mesh stencil_mesh;

struct Portal
{
    vec3 position; // Position of portal origin relative world origin
    mat3 rotation; // Rotation from portal frame to world frame
                   // (portal frame expressed in world frame)
    Portal *link;
};

Portal portal_a;
Portal portal_b;
Portal portals[2];

// R: Rotation from view frame to world frame
// p: Camera position from world origin relative world frame
mat4 view_from_se3(mat3 R, vec3 p)
{
    mat3 T = m_transpose(R);
    return m_se3(T, -T*p);
}

void se3_decompose(mat4 se3, mat3 *R, vec3 *p)
{
    *R = m_mat3(se3);
    *p = se3.a4.xyz;
}

void init()
{
    stencil_mesh = make_cube();
    world = load_mesh("assets/models/sdftest/sdftest.sumo_asset", MESH_NORMALIZE);
    diffuse = so_load_tex2d("assets/models/sdftest/diffuse.png");
    renders.diffuse = load_render_pass("assets/shaders/diffuse.vs", "assets/shaders/diffuse.fs");
    renders.stencil = make_render_pass(stencil_vs, stencil_fs);

    portal_a.position = m_vec3(0.0f, -0.05f, 0.05f);
    portal_a.rotation = m_mat3(mat_rotate_y(0.0f));
    portal_a.link = &portal_b;

    portal_b.position = m_vec3(-0.05f, -0.05f, -0.2f);
    portal_b.rotation = m_mat3(mat_rotate_y(0.5f));
    portal_b.link = &portal_a;

    portals[0] = portal_a;
    portals[1] = portal_b;
}

void draw_world_no_portals(mat4 projection,
                           mat4 view)
{
    begin(&renders.diffuse);
    uniformf("projection", projection);
    uniformf("model", mat_scale(1.0f) * mat_translate(0.0f, -0.2f, 0.0f));
    uniformf("view", view);
    uniformf("albedo", m_vec4(1.0f, 1.0f, 1.0f, 1.0f));
    uniformi("use_vertex_color", 0);
    uniformi("use_diffuse", 1);
    uniformi("diffuse", 0);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glBindBuffer(GL_ARRAY_BUFFER, world.positions);
    attribfv("position", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, world.normals);
    attribfv("normal", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, world.texels);
    attribfv("texel", 2, 2, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world.indices);
    glDrawElements(GL_TRIANGLES, world.num_indices, GL_UNSIGNED_INT, 0);
}

void draw_portal_overlay(mat4 projection, mat4 view)
{
    // Draw portal overlay
    glBindBuffer(GL_ARRAY_BUFFER, stencil_mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stencil_mesh.ibo);
    attribfv("position", 3, 6, 0);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("albedo", m_vec4(0.5f, 0.8f, 1.0f, 0.2f));
    uniformi("use_vertex_color", 0);
    uniformi("use_diffuse", 0);
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        mat4 model = m_se3(portals[portal_index].rotation,
                           portals[portal_index].position);
        uniformf("model", model * mat_scale(0.005f, 0.06f, 0.04f));
        glDrawElements(GL_TRIANGLES, stencil_mesh.index_count, stencil_mesh.index_type, 0);
    }
}

void draw_world(mat4 projection, mat4 view)
{
    mat3 camera_rotation; // Camera frame relative world frame
    vec3 camera_position; // Camera position relative world origin
    {
        mat3 R;
        vec3 p;
        se3_decompose(view, &R, &p);
        camera_rotation = m_transpose(R);
        camera_position = -camera_rotation * p;
    }

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    blend_mode(true);
    depth_write(true);
    depth_test(true);

    begin(&renders.stencil);
    glBindBuffer(GL_ARRAY_BUFFER, stencil_mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stencil_mesh.ibo);
    attribfv("position", 3, 6, 0);
    uniformf("projection", projection);
    uniformf("view", view);

    glStencilMask(0xFF);
    glClearStencil(0);
    glClearColor(0.15f, 0.1f, 0.05f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    depth_write(false);
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        mat4 model = m_se3(portals[portal_index].rotation,
                           portals[portal_index].position);
        glStencilFunc(GL_ALWAYS, portal_index + 1, 0xFF);
        uniformf("model", model * mat_scale(0.005f, 0.06f, 0.04f));
        glDrawElements(GL_TRIANGLES, stencil_mesh.index_count, stencil_mesh.index_type, 0);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    depth_write(true);

    glStencilMask(0x00);
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        Portal a = portals[portal_index];
        Portal b = *a.link;
        mat3 Rac = m_transpose(a.rotation) * camera_rotation;
        mat3 Rwc = b.rotation * Rac;
        vec3 pac = camera_position - a.position;
        vec3 pwc = b.rotation * pac;
        mat4 portal_view = view_from_se3(Rwc, pwc);
        glStencilFunc(GL_EQUAL, portal_index + 1, 0xFF);
        draw_world_no_portals(projection, portal_view);
    }

    glStencilFunc(GL_EQUAL, 0, 0xFF);
    draw_world_no_portals(projection, view);
    glDisable(GL_STENCIL_TEST);
    draw_portal_overlay(projection, view);
    /*
    1) Draw portal stencils
        No portal -> 0
        Portal a -> 1
        Portal b -> 2

        mat3 Rvw;
        vec3 pwvm;
        se3_decompose(view, &Rvw, &pwvm);
        mat3 camera_rotation = m_transpose(Rvw);
        vec3 camera_position = -camera_rotation * pwvm;
    2) if recursion < recursion limit
            foreach portal:
                glStencilFunc(GL_EQUAL, i+1, 0xFF)
                mat3 Rac = portal.rotation' * camera.rotation
                mat3 Rwc = portal.link->rotation * Rac
                vec3 pac = camera.position - portal.position
                vec3 pwc = portal.link->rotation * pac;
                view_from_se3(mat3 R, vec3 p)
                portal_view = ...
                draw_world_no_portals(projection, portal_view)
    3)
        glStencilFunc(GL_EQUAL, 0, 0xFF)
        draw_world_no_portals(projection, view)
    */
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 3.5f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.02f, 10.0f);
    mat4 view = camera_fps(io, dt, 0.2f);

    draw_world(projection, view);
}

#include "sumo.cpp"
