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

mat4 stateless_camera_fps(quat *q, vec3 *p,
                          Input io, float dt,
                          float movespeed = 1.0f,
                          float sensitivity = 1.0f)
{
    mat3 R = m_quat_to_so3(*q);
    float wx = 0.0f;
    float wy = 0.0f;
    if (io_key_down(UP))         wx = +1.0f;
    else if (io_key_down(DOWN))  wx = -1.0f;
    if (io_key_down(LEFT))       wy = +1.0f;
    else if (io_key_down(RIGHT)) wy = -1.0f;

    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
    if (io_key_down(D))          vx = -1.0f;
    else if (io_key_down(A))     vx = +1.0f;
    if (io_key_down(W))          vz = +1.0f;
    else if (io_key_down(S))     vz = -1.0f;
    if (io_key_down(SPACE))      vy = +1.0f;
    else if (io_key_down(LCTRL)) vy = -1.0f;

    vec3 dp = R*m_vec3(vx, vy, vz);
    vec3 w = m_vec3(0, wy, 0) + R.a1*wx;
    quat dq = 0.5f * m_quat_mul(m_vec4(w, 0.0f), *q);

    *q += dq * dt;
    *q = m_normalize(*q);
    *p += dp * dt;

    return view_from_se3(R, *p);
}

#if 0
mat4 camera_fps_collision(quat *t, vec3 *p,
                          Input io, float dt,
                          vec4 *plane_equations,
                          vec2 *plane_dimensions,
                          int num_planes,
                          float movespeed = 1.0f,
                          float sensitivity = 1.0f)
{
    mat3 R = m_quat_to_so3(*q);
    float wx = 0.0f;
    float wy = 0.0f;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    float kw = 2.0f * PI / (0.1f * WINDOW_WIDTH);

    float wy = -kw * io.mouse.rel.x * sensitivity;
    float wy = kw * io.mouse.rel.x * sensitivity;

    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
    if (io_key_down(D))          vx = -1.0f;
    else if (io_key_down(A))     vx = +1.0f;
    if (io_key_down(W))          vz = +1.0f;
    else if (io_key_down(S))     vz = -1.0f;
    if (io_key_down(SPACE))      vy = +1.0f;
    else if (io_key_down(LCTRL)) vy = -1.0f;

    vec3 dp = R*m_vec3(vx, vy, vz);
    vec3 w = m_vec3(0, wy, 0) + R.a1*wx;
    quat dq = 0.5f * m_quat_mul(m_vec4(w, 0.0f), *q);

    *q += dq * dt;
    *q = m_normalize(*q);
    *p += dp * dt;

    return view_from_se3(R, *p);
}
#endif

// Project a point w in world-space into NDC
vec2 project(mat4 projection, mat4 view, vec3 w)
{
    vec4 clip = projection * view * m_vec4(w, 1.0f);
    if (clip.w > 0.01f)
        return clip.xy / clip.w;
    else
        return clip.xy  / 0.01f;
}

#include <stdio.h>
void lines_draw_3d(mat4 projection, mat4 view, vec3 a, vec3 b)
{
    vec2 ap = project(projection, view, a);
    vec2 ab = project(projection, view, b);
    lines_draw_line(ap, ab);
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);

    // One line to a) Set initial position/orientation, and
    //             b) Provide "memory" for camera
    // Pretty slick!
    persist vec3 p = m_vec3(0.1f, 0.1f, 2.0f);
    persist quat q = m_quat_from_angle_axis(m_vec3(0,1,0), PI);
    mat4 view = stateless_camera_fps(&q, &p, io, dt);

    blend_mode(true);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    s32 n = 32;
    lines_set_color(0x63636377);
    for (s32 i = 0; i < n; i++)
    {
        float a = -1.0f + 2.0f * i / (float)(n-1);
        lines_draw_3d(projection, view, m_vec3(1, 0, a), m_vec3(-1, 0, a));
        lines_draw_3d(projection, view, m_vec3(a, 0, 1), m_vec3(a, 0, -1));
    }
    lines_set_width(2.0f);
    lines_set_color(0xB23F24FF);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(1, 0, 0));
    lines_set_color(0x78B041FF);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(0, 1, 0));
    lines_set_color(0x3490E0FF);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(0, 0, 1));
    lines_flush();
}

#include "sumo.cpp"
