#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
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

// Some notation:
// The camera frame consists of three axes: x, y and z.
// The camera's forward vector, however, negative z, due
// to convention of the projection matrix. That is why
// I multiply by -1 in the below functions (it's actually
// a 180 degree rotation about the y axis).

// R: Rotation from view frame to world frame
// p: Camera position from world origin relative world frame
mat4 view_from_se3(mat3 R, vec3 p)
{
    R.a1 *= -1.0f;
    R.a3 *= -1.0f;
    mat3 T = m_transpose(R);
    return m_se3(T, -T*p);
}

void se3_decompose(mat4 se3, mat3 *R, vec3 *p)
{
    *R = m_mat3(se3);
    *p = se3.a4.xyz;
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
    R_w.a1 *= -1.0f;
    R_w.a3 *= -1.0f;
    *R = R_w; // Camera frame relative world frame
    *p = p_w; // Camera position relative world origin
}

void init()
{
    stencil_mesh = make_cube();
    portal_mesh = make_cube();
    world = load_mesh("assets/models/sdftest/sdftest.sumo_asset", MESH_NORMALIZE);
    diffuse = so_load_tex2d("assets/models/sdftest/diffuse.png");
    renders.diffuse = load_render_pass("assets/shaders/diffuse.vs", "assets/shaders/diffuse.fs");
    renders.stencil = make_render_pass(stencil_vs, stencil_fs);

    portal_a.position = m_vec3(0.2f, 0.0f, 0.2f);
    portal_a.rotation = m_mat3(mat_rotate_y(1.7f) * mat_rotate_x(0.5f));
    portal_a.scale = m_vec3(0.08f, 0.08f, 0.005f);
    portal_a.link = &portal_b;

    portal_b.position = m_vec3(-0.2f, 0.0f, 0.0f);
    portal_b.rotation = m_mat3(mat_rotate_y(-0.2f));
    portal_b.scale = m_vec3(0.08f, 0.08f, 0.005f);
    portal_b.link = &portal_a;

    portals[0] = portal_a;
    portals[1] = portal_b;

    lines_init();
}

vec4 plane_equation_from_portal(Portal p)
{
    vec4 result;
    result.xyz = p.rotation.a3;
    result.w = -m_dot(p.rotation.a3, p.position);
    return result;
}

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

    vec4 colors[array_count(portals)] = {
        m_vec4(0.8f, 0.31f, 0.2f, 1.0f),
        m_vec4(0.8f, 0.31f, 0.2f, 1.0f)
    };
    for (u32 i = 0; i < array_count(portals); i++)
    {
        uniformf("model", portals[i].model * mat_translate(0.0f, 0.0f, -6.0f) * mat_scale(1.2f, 1.2f, 5.0f));
        uniformf("albedo", colors[i]);
        uniformi("use_vertex_color", 0);
        uniformi("use_diffuse", 0);
        glDrawElements(GL_TRIANGLES, portal_mesh.index_count, portal_mesh.index_type, 0);
    }
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
    glClearColor(0.85f, 0.82f, 0.72f, 1.0f);
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
    glDisable(GL_STENCIL_TEST);
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 3.5f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.02f, 10.0f);
    mat4 view = camera_fps(io, dt, 0.2f);

    mat3 camera_rotation;
    vec3 camera_position;
    decompose_view(view, &camera_rotation, &camera_position);
    for (u32 i = 0; i < array_count(portals); i++)
    {
        mat3 B = portals[i].link->rotation * m_mat3(mat_rotate_y(PI));
        Portal a = portals[i];
        Portal b = *a.link;
        mat3 T = B * m_transpose(a.rotation);
        mat3 R = T * camera_rotation;
        vec3 p = T * (camera_position - a.position) + b.position;
        portals[i].view = view_from_se3(R, p);
        portals[i].model = m_se3(a.rotation * m_mat3(mat_scale(a.scale)), a.position);
    }

    // Debugging
    if (io_key_down(1))
        view = portals[0].view;
    else if (io_key_down(2))
        view = portals[1].view;

    draw_world(io, projection, view);
}

#include "sumo.cpp"
