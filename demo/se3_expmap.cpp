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
    if (io_key_down(UP))         wx = -1.0f;
    else if (io_key_down(DOWN))  wx = +1.0f;
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

// Project a point w in world-space into NDC
vec2 project(mat4 projection, mat4 view, vec3 w)
{
    vec4 clip = projection * view * m_vec4(w, 1.0f);
    if (clip.w > 0.01f)
        return clip.xy / clip.w;
    else
        return clip.xy  / 0.01f;
}

void lines_draw_3d(mat4 projection, mat4 view, vec3 a, vec3 b)
{
    vec2 ap = project(projection, view, a);
    vec2 ab = project(projection, view, b);
    lines_draw_line(ap, ab);
}

mat3 so3_exp(vec3 wt)
{
    r32 t = m_length(wt);
    if (t < 0.01f)
    {
        return m_id3() + m_skew(wt);
    }
    else
    {
        mat3 W = m_skew(wt/t);
        return m_id3() + sin(t)*W + (1.0f-cos(t))*W*W;
    }
}

// https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation#Log_map_from_SO.283.29_to_so.283.29
vec3 so3_log(mat3 R)
{
    r32 tr = R.a11+R.a22+R.a33;
    r32 theta = acos((tr-1.0f)/2.0f);
    if (theta < 0.01f)
    {
        // For small angles
        // R = I + theta K = I + W
        vec3 w;
        w.x = R.a32;
        w.y = R.a13;
        w.z = R.a21;
        return w;
    }
    else
    {
        vec3 k;
        r32 s = sin(theta);
        k.x = (R.a32-R.a23)/(2.0f*s);
        k.y = (R.a13-R.a31)/(2.0f*s);
        k.z = (R.a21-R.a12)/(2.0f*s);
        vec3 w = k*theta;
        return w;
    }
}

// Motilal Agrawal.
// A Lie Algebraic Approach for Consistent Pose Registration for General Euclidean Motion.
// Proceedings of the 2006 IEEE/RSJ International Conference on Intelligent Robots and Systems
void se3_log(mat4 SE3, vec3 *out_w, vec3 *out_v)
{
    mat3 R;
    vec3 T;
    se3_decompose(SE3, &R, &T);
    vec3 w = so3_log(R);

    r32 t = m_length(w);
    if (t < 0.01f)
    {
        *out_v = T;
    }
    else
    {
        mat3 W = m_skew(w);
        r32 s = sin(t);
        r32 c = cos(t);
        mat3 M = m_id3() - 0.5f*W + W*W*((2.0f*s - t*(1.0f+c))/(2.0f*t*t*s));
        *out_v = M*T;
    }

    *out_w = w;
}

mat4 se3_exp(vec3 w, vec3 v)
{
    mat3 R = so3_exp(w);
    r32 t = m_length(w);
    vec3 T = v;
    if (t >= 0.01f)
    {
        vec3 a = w / t;
        mat3 A = m_skew(a);
        T = v + (A*(1.0f-cos(t)) + A*A*(t-sin(t)))*(v/t);

        // This is equivalent
        // T = m_outer_product(a, a)*v + ((m_id3()-R)*m_skew(a))*(v/t);
    }
    mat4 result = m_se3(R, T);
    return result;
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
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
    lines_set_color(0xB23F2455);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(1, 0, 0));
    lines_set_color(0x78B04155);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(0, 1, 0));
    lines_set_color(0x3490E055);
    lines_draw_3d(projection, view, m_vec3(0), m_vec3(0, 0, 1));

    // Draw E frame
    #if 0
    persist vec3 wt = m_vec3(0.0f, 0.0f, 0.0f);
    persist vec3 vt = m_vec3(0.0f, 0.0f, 0.0f);
    vec3 wt_check;
    vec3 vt_check;
    {
        mat4 E = se3_exp(wt, vt);
        mat3 R;
        vec3 p;
        m_se3_decompose(E, &R, &p);
        lines_set_color(0xB23F24FF);
        lines_draw_3d(projection, view, p, p + R.a1);
        lines_set_color(0x78B041FF);
        lines_draw_3d(projection, view, p, p + R.a2);
        lines_set_color(0x3490E0FF);
        lines_draw_3d(projection, view, p, p + R.a3);

        se3_log(E, &wt_check, &vt_check);
    }
    lines_flush();
    ImGui::NewFrame();
    ImGui::Begin("SE3");
    ImGui::SliderFloat("wx", &wt.x, -2.0f, 2.0f);
    ImGui::SliderFloat("wy", &wt.y, -2.0f, 2.0f);
    ImGui::SliderFloat("wz", &wt.z, -2.0f, 2.0f);
    ImGui::SliderFloat("vx", &vt.x, -2.0f, 2.0f);
    ImGui::SliderFloat("vy", &vt.y, -2.0f, 2.0f);
    ImGui::SliderFloat("vz", &vt.z, -2.0f, 2.0f);
    ImGui::Text("w = (%.6f %.6f %.6f)\nw = (%.6f %.6f %.6f)", wt.x, wt.y, wt.z, wt_check.x, wt_check.y, wt_check.z);
    ImGui::Text("v = (%.6f %.6f %.6f)\nv = (%.6f %.6f %.6f)", vt.x, vt.y, vt.z, vt_check.x, vt_check.y, vt_check.z);
    ImGui::End();
    ImGui::Render();
    #endif

    #if 1
    mat4 H0 = mat_translate(0.0f, 0.0f, 0.0f) * mat_rotate_x(0.0f) * mat_rotate_y(0.0f) * mat_rotate_z(0.0f);
    mat4 H1 = mat_translate(1.0f, 1.0f, 1.0f) * mat_rotate_x(0.0f) * mat_rotate_y(PI/2.0f) * mat_rotate_z(PI/2.0f);
    vec3 w, v; se3_log(H1*m_se3_inverse(H0), &w, &v);
    static r32 tf = 0.0f;
    int nt = (int)(tf * 24.0f)+1;
    for (int i = 0; i <= nt; i++)
    {
        r32 t = tf * i / nt;
        if (t > tf) t = tf;
        mat4 Ht = se3_exp(w*t, v*t)*H0;
        mat3 R;
        vec3 p;
        m_se3_decompose(Ht, &R, &p);
        lines_set_color(0xB23F24FF);
        lines_draw_3d(projection, view, p, p + R.a1);
        lines_set_color(0x78B041FF);
        lines_draw_3d(projection, view, p, p + R.a2);
        lines_set_color(0x3490E0FF);
        lines_draw_3d(projection, view, p, p + R.a3);
    }
    lines_flush();
    ImGui::NewFrame();
    ImGui::Begin("SE3");
    ImGui::SliderFloat("t", &tf, 0.0f, 1.0f);
    ImGui::End();
    ImGui::Render();
    #endif
}

#include "sumo.cpp"
