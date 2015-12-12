#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void init()
{
    lines_init();
}

// R: Rotation from view frame to world frame
// p: Camera position from world origin relative world frame
mat4 view_from_se3(mat3 R, vec3 p)
{
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
    R_w.a3 *= -1.0f;
    *R = R_w; // Camera frame relative world frame
    *p = p_w; // Camera position relative world origin
}

mat4 stateless_camera_fps(mat4 *view, Input io, float dt,
                          float movespeed, float sensitivity)
{
    // TODO: implement
}

void tick(Input io, float t, float dt)
{
    persist quat view_q = m_quat_from_angle_axis(m_vec3(0.0f, 1.0f, 0.0f), 0.0f);
    mat3 view_R = m_quat_to_so3(view_q);
    vec3 view_p = m_vec3(0.2f, 0.2f, 0.44f);
    mat4 view = view_from_se3(view_R, view_p);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    persist r32 theta = 0.0f;
    persist r32 dtheta = 0.0f;
    persist r32 phi = 0.0f;
    persist r32 dphi = 0.0f;
    r32 ktheta = 2.0f * PI / (0.1f * WINDOW_WIDTH);
    r32 kphi = 2.0f * PI / (0.1f * WINDOW_WIDTH);
    dtheta = ktheta * io.mouse.rel.x * 3.0f;
    dphi = kphi * io.mouse.rel.y * 3.0f;
    theta += dt * dtheta;
    phi += dt * dphi;

    view_q += 0.5f * m_quat_mul(view_q, m_vec4(-dphi, 0.0f, 0.0f, 0.0f)) * dt;
    view_q = m_normalize(view_q);
    view_q += 0.5f * m_quat_mul(m_vec4(0.0f, -dtheta, 0.0f, 0.0f), view_q) * dt;
    view_q = m_normalize(view_q);

    mat3 camera_rotation;
    vec3 camera_position;
    decompose_view(view, &camera_rotation, &camera_position);

    clearc(0.35f, 0.55f, 1.0f, 1.0f);

    mat3 world_to_window = m_mat3(m_vec3(1, 0, 0),
                        m_vec3(0, 0, 1),
                        m_vec3(0, 1, 0));
    vec3 forward = m_vec3(0.0f, 0.0f, 0.5f);
    forward = world_to_window * camera_rotation * forward;
    depth_test(false);
    depth_write(false);
    lines_set_color(0x000000ff);
    lines_draw_line(0.0f, 0.0f, 0.0f, 0.5f); // z-axis
    lines_draw_line(0.0f, 0.0f, 0.5f, 0.0f); // x-axis
    lines_draw_line(0.0f, 0.0f, forward.x, forward.y);
    lines_flush();
}

#include "sumo.cpp"
