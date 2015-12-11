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
Mesh portal_mesh;

struct Portal
{
    vec3 position; // Position of portal origin relative world origin
    mat3 rotation; // Rotation from portal frame to world frame
                   // (portal frame expressed in world frame)
    vec3 scale;
    Portal *link;

    mat4 model; // Computed object->world transformation matrix
    mat4 view;  // Computed world->view transformation matrix
                // to be used when rendering the world seen
                // _through_ this portal (that is, the world
                // seen from the linked portal).
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
    portal_mesh = make_cube();
    world = load_mesh("assets/models/sdftest/sdftest.sumo_asset", MESH_NORMALIZE);
    diffuse = so_load_tex2d("assets/models/sdftest/diffuse.png");
    renders.diffuse = load_render_pass("assets/shaders/diffuse.vs", "assets/shaders/diffuse.fs");
    renders.stencil = make_render_pass(stencil_vs, stencil_fs);

    portal_a.position = m_vec3(0.1f, 0.0f, -0.1f);
    portal_a.rotation = m_mat3(mat_rotate_y(0.0f));
    portal_a.scale = m_vec3(0.08f, 0.08f, 0.005f);
    portal_a.link = &portal_b;

    portal_b.position = m_vec3(-0.1f, 0.0f, 0.1f);
    portal_b.rotation = m_mat3(mat_rotate_y(0.5f));
    portal_b.scale = m_vec3(0.08f, 0.08f, 0.005f);
    portal_b.link = &portal_a;

    portals[0] = portal_a;
    portals[1] = portal_b;
}

vec4 plane_equation_from_portal(Portal p)
{
    vec4 result;
    result.xyz = -p.rotation.a3;
    result.w = m_dot(p.rotation.a3, p.position);
    return result;
}

#if 1
void draw_portal_stencils(mat4 projection,
                          mat4 view)
{
    begin(&renders.stencil);
    glBindBuffer(GL_ARRAY_BUFFER, stencil_mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stencil_mesh.ibo);
    attribfv("position", 3, 6, 0);
    uniformf("projection", projection);
    uniformf("view", view);
    for (u32 i = 0; i < array_count(portals); i++)
    {
        glStencilFunc(GL_ALWAYS, i + 1, 0xFF);
        uniformf("model", portals[i].model);
        glDrawElements(GL_TRIANGLES, stencil_mesh.index_count, stencil_mesh.index_type, 0);
    }
}

void draw_level(mat4 projection, mat4 view, vec4 clip)
{
    begin(&renders.diffuse);
    uniformf("projection", projection);
    uniformf("model", mat_scale(1.0f) * mat_translate(0.0f, -0.2f, 0.0f));
    uniformf("view", view);
    uniformf("albedo", m_vec4(1.0f, 1.0f, 1.0f, 1.0f));
    uniformi("use_vertex_color", 0);
    uniformi("use_diffuse", 1);
    uniformi("diffuse", 0);
    uniformf("clip0", clip);
    glBindTexture(GL_TEXTURE_2D, diffuse);
    glBindBuffer(GL_ARRAY_BUFFER, world.positions);
    attribfv("position", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, world.normals);
    attribfv("normal", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, world.texels);
    attribfv("texel", 2, 2, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world.indices);
    glDrawElements(GL_TRIANGLES, world.num_indices, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, portal_mesh.vbo);
    attribfv("position", 3, 6, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, portal_mesh.ibo);

    // int i = 0;
    // uniformf("model", portals[i].model * mat_translate(0.0f, 0.0f, +6.0f) * mat_scale(1.2f, 1.2f, 5.0f));
    // uniformf("albedo", m_vec4(0.32f, 0.13f, 0.1f, 1.0f));
    // uniformi("use_vertex_color", 0);
    // uniformi("use_diffuse", 0);
    // glDrawElements(GL_TRIANGLES, portal_mesh.index_count, portal_mesh.index_type, 0);

    // i = 1;
    // uniformf("model", portals[i].model * mat_translate(0.0f, 0.0f, -6.0f) * mat_scale(1.2f, 1.2f, 5.0f));
    // uniformf("albedo", m_vec4(0.22f, 0.36f, 0.42f, 1.0f));
    // uniformi("use_vertex_color", 0);
    // uniformi("use_diffuse", 0);
    // glDrawElements(GL_TRIANGLES, portal_mesh.index_count, portal_mesh.index_type, 0);
}

// Decomposes the (world->view) transformation into
// a rotation matrix and position vector describing
// the camera rotation and position relative to the
// world frame.
void decompose_view(mat4 view, mat3 *R, vec3 *p)
{
    mat3 R_c, R_w;
    vec3 p_c, p_w;
    se3_decompose(view, &R_c, &p_c);
    R_w = m_transpose(R_c);
    p_w = -R_w * p_c;
    *R = R_w; // Camera frame relative world frame
    *p = p_w; // Camera position relative world origin
}

void draw_world(Input io, mat4 projection, mat4 view)
{
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glDepthFunc(GL_LEQUAL);

    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0xFF);
    glClearColor(0.2f, 0.35f, 0.8f, 1.0f);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    draw_portal_stencils(projection, view);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    draw_level(projection, view, m_vec4(0));

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CLIP_DISTANCE0);
    for (u32 i = 0; i < array_count(portals); i++)
    {
        glStencilFunc(GL_EQUAL, i + 1, 0xFF);
        vec4 clip = plane_equation_from_portal(*portals[i].link);
        draw_level(projection, portals[i].view, clip);
    }
    glDisable(GL_CLIP_DISTANCE0);
}
#else
void draw_world_no_portals(mat4 projection,
                           mat4 view)
{
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
    uniformi("use_vertex_color", 0);
    uniformi("use_diffuse", 0);
    vec4 colors[array_count(portals)] = {
        m_vec4(1.0f, 0.2f, 0.1f, 0.1f),
        m_vec4(0.1f, 0.2f, 1.0f, 0.1f)
    };
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        glStencilFunc(GL_EQUAL, portal_index + 1, 0xFF);
        uniformf("albedo", colors[portal_index]);
        uniformf("model", portals[portal_index].model);
        glDrawElements(GL_TRIANGLES, stencil_mesh.index_count, stencil_mesh.index_type, 0);
    }
}

void draw_world(Input io, mat4 projection, mat4 view)
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

    // Compute portal-relative viewports
    mat4 portal_views[array_count(portals)];
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        Portal a = portals[portal_index];
        Portal b = *a.link;
        mat3 R = b.rotation * m_transpose(a.rotation) * camera_rotation;
        vec3 p = b.rotation * m_transpose(a.rotation) * (camera_position - a.position) + b.position;
        mat4 portal_view = view_from_se3(R, p);
        portal_views[portal_index] = portal_view;

        // And the model transformation matrix while we're at it
        mat4 model = m_se3(portals[portal_index].rotation,
                           portals[portal_index].position);
        portals[portal_index].model =
                    model * mat_scale(portals[portal_index].scale);
    }

    // Debugging
    if (io_key_down(1))
        view = portal_views[0];
    else if (io_key_down(2))
        view = portal_views[1];

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    blend_mode(true);
    depth_write(true);
    depth_test(true);
    glStencilMask(0xFF);
    glClearStencil(0);
    glClearColor(0.15f, 0.1f, 0.05f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0x00);
    begin(&renders.diffuse);
    draw_world_no_portals(projection, view);

    begin(&renders.stencil);
    glBindBuffer(GL_ARRAY_BUFFER, stencil_mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, stencil_mesh.ibo);
    attribfv("position", 3, 6, 0);
    uniformf("projection", projection);
    uniformf("view", view);
    glStencilMask(0xFF);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    depth_write(false);
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        glStencilFunc(GL_ALWAYS, portal_index + 1, 0xFF);
        uniformf("model", portals[portal_index].model);
        glDrawElements(GL_TRIANGLES, stencil_mesh.index_count, stencil_mesh.index_type, 0);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    depth_write(true);
    glClear(GL_DEPTH_BUFFER_BIT);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);
    begin(&renders.diffuse);
    glEnable(GL_CLIP_DISTANCE0);
    for (u32 portal_index = 0;
         portal_index < array_count(portals);
         portal_index++)
    {
        uniformf("clip0", plane_equation_from_portal(*portals[portal_index].link));
        glStencilFunc(GL_EQUAL, portal_index + 1, 0xFF);
        draw_world_no_portals(projection, portal_views[portal_index]);
        uniformf("clip0", m_vec4(0));
    }
    glDisable(GL_CLIP_DISTANCE0);

    draw_portal_overlay(projection, view);
}
#endif

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 3.5f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.02f, 10.0f);
    mat4 view = camera_fps(io, dt, 0.2f);

    // mat4 view = mat_translate(-0.06f, 0.0f, -0.4f) * mat_rotate_x(0.3f) * mat_rotate_y(0.23f);

    mat3 camera_rotation;
    vec3 camera_position;
    decompose_view(view, &camera_rotation, &camera_position);
    for (u32 i = 0; i < array_count(portals); i++)
    {
        Portal a = portals[i];
        Portal b = *a.link;
        mat3 T = b.rotation * m_transpose(a.rotation);
        mat3 R = T * camera_rotation;
        vec3 p = T * (camera_position - a.position) + b.position;
        portals[i].view = view_from_se3(R, p);
        portals[i].model = m_se3(a.rotation, a.position) * mat_scale(a.scale);
    }

    // Debugging
    if (io_key_down(1))
        view = portals[0].view;
    else if (io_key_down(2))
        view = portals[1].view;

    draw_world(io, projection, view);
}

#include "sumo.cpp"
