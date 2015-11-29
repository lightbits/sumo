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

#define USE_NEW_MATH
#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4

RenderPass pass;
MeshAsset mesh_quad;
GLuint tex_quad;

#define NUM_PROPS 4
struct State
{
    // Constant parameters
    mat3 I;   // Inertia matrix in {b} about center of gravity
              // Assumed to be diagonal
    mat3 M;   // Mass matrix in {n} about center of gravity
              // Assumed to be diagonal

    // Time-varying quantities
    vec3 p_n; // Position in inertial frame
    vec3 v_n; // Velocity in inertial frame
    quat q;   // Orientation unit quaternion
    quat qd;  // Desired orientation (TODO: DEBUG, remove)
    vec3 w_b; // Angular velocity in body frame

    // System inputs
    struct Prop
    {
        // Input
        float u; // Applied voltage (volts)
        float s; // Direction (CCW:+, CW:-)

        // Parameters
        vec3  r; // Arm from center of gravity to Prop center (meters)
        float Kv; // Motor coefficient (rad/s per volt)
        float k; // Motor coefficient (Force per square rad/s)
        float b; // Torque coefficient (Torque per square rad/s)

        // Computed parameters
        float w; // Angular speed
    };
    Prop props[NUM_PROPS];
};

// This implements the Newton-Euler equations of motion
void ode_integrate(State *state, float dt)
{
    // Compute the rotation matrix transforming
    // body coordinates into inertial coordinates (R^n_b)
    mat3 R = m_quat_to_so3(state->q);

    // Inverse mass matrix
    mat3 M_inv = m_id3();
    M_inv.a11 = 1.0f/state->M.a11;
    M_inv.a22 = 1.0f/state->M.a22;
    M_inv.a33 = 1.0f/state->M.a33;

    // Inverse inertia matrix
    mat3 I_inv = m_id3();
    I_inv.a11 = 1.0f/state->I.a11;
    I_inv.a22 = 1.0f/state->I.a22;
    I_inv.a33 = 1.0f/state->I.a33;

    // Compute forces and torques
    vec3 f_n = m_vec3(0.0f); // Linear forces in inertial frame
    vec3 t_b = m_vec3(0.0f); // Torque forces in body frame
    for (u32 i = 0; i < NUM_PROPS; i++)
    {
        float u = state->props[i].u;
        float s = state->props[i].s;

        float Kv = state->props[i].Kv;
        float k = state->props[i].k;
        float b = state->props[i].b;
        vec3  r = state->props[i].r;

        float w = Kv * u;
        float f = k*w*w;
        f_n += R.a3*f;
        t_b.z += -b*s*w*w;
        t_b += m_cross(r, m_vec3(0.0f, 0.0f, f));

        state->props[i].w = w;
    }
    // Control law (TODO: Optimal control allocation using propellers)
    mat3 Kd = state->I * TWO_PI;
    mat3 Kp = 10.0f*Kd;
    quat qd = state->qd;
    Matrix<float,4,3> T = m_quat_mul_matrix(state->q);
    t_b = - m_cross(state->I*state->w_b, state->w_b)
          - Kd*state->w_b
          - Kp*0.5f*(m_transpose(T)*(state->q-qd));

    f_n += state->M*m_vec3(0.0f, 0.0f, -9.81f);

    // Dynamic differential equations
    vec3 Dp_n = state->v_n;
    vec3 Dv_n = M_inv*f_n;
    vec3 Dw_b = I_inv*(t_b+m_cross(state->I*state->w_b, state->w_b));

    // Kinematic differential equation
    quat Dq = 0.5f * T * state->w_b;

    // Forward Euler integration
    state->p_n += Dp_n*dt;
    state->v_n += Dv_n*dt;
    state->w_b += Dw_b*dt;
    state->q   += Dq*dt;

    // Todo: Dynamic normalization?
    state->q = m_normalize(state->q);
}

static State state;

#define Log_Entries 256
struct History
{
    float qx[Log_Entries];
    float qy[Log_Entries];
    float qz[Log_Entries];
    float qw[Log_Entries];
    float t[Log_Entries];
};

History history;
u32 log_index;

void init()
{
    // Length from body frame origin to each propeller
    float L1 = 0.25f;
    float L2 = 0.25f;
    float L3 = 0.25f;
    float L4 = 0.25f;

    float point_mass = 0.52f/4.0f;

    state.I = m_id3();
    state.I.a11 = point_mass*(L2*L2+L4*L4);
    state.I.a22 = point_mass*(L1*L3+L1*L3);
    state.I.a33 = point_mass*(L1*L1+L2*L2+L3*L3+L4*L4);

    state.M = m_id3();
    state.M.a11 = 0.52f;
    state.M.a22 = 0.52f;
    state.M.a33 = 0.52f;

    state.p_n = m_vec3(0.0f, 0.0f, 0.0f);
    state.v_n = m_vec3(0.0f, 0.0f, 0.0f);
    state.w_b = m_vec3(0.0f, 0.0f, 0.0f);
    state.q = m_quat_from_angle_axis(m_vec3(0.0f, 0.0f, 1.0f), PI/4.0f);
    // state.q = m_vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // See i.e. http://blog.oscarliang.net/how-to-choose-motor-and-propeller-for-quadcopter/
    // for coefficients
    float CD = 0.5f;
    float rho = 1.225f;
    float R = 0.1f;
    float A = R*0.03f;
    float g = 9.81f;

    State::Prop base_prop = {};
    base_prop.u = 0.0f;
    base_prop.s = +1.0f;
    base_prop.Kv = 2000.0f*TWO_PI/60.0f;
    base_prop.k = 0.13f*g/m_square(5000.0f*TWO_PI/60.0f);
    base_prop.b = 0.5f*R*R*R*rho*CD*A;
    base_prop.w = 0.0f;

    state.props[0] = base_prop;
    state.props[0].r = m_vec3(1.0f, 0.0f, 0.0f);
    state.props[0].s = +1.0f;

    state.props[1] = base_prop;
    state.props[1].r = m_vec3(0.0f, 1.0f, 0.0f);
    state.props[1].s = -1.0f;

    state.props[2] = base_prop;
    state.props[2].r = m_vec3(-1.0f, 0.0f, 0.0f);
    state.props[2].s = +1.0f;

    state.props[3] = base_prop;
    state.props[3].r = m_vec3(0.0f, -1.0f, 0.0f);
    state.props[3].s = -1.0f;

    pass = load_render_pass("assets/shaders/lowpoly.vs",
                            "assets/shaders/lowpoly.fs");
    mesh_quad = load_mesh("assets/models/quadcopter/quadcopter.sumo_asset");
    tex_quad = so_load_tex2d("assets/models/quadcopter/quadcopter.png", 0, 0,
                             GL_NEAREST, GL_NEAREST);

    log_index = 0;
    memset(history.qx, 0, sizeof(history.qx));
    memset(history.qy, 0, sizeof(history.qy));
    memset(history.qz, 0, sizeof(history.qz));
    memset(history.qw, 0, sizeof(history.qw));
    memset(history.t, 0, sizeof(history.t));
}

void add_history_entry(quat q, float t)
{
    if (log_index == Log_Entries)
        return;
    history.qx[log_index] = q.x;
    history.qy[log_index] = q.y;
    history.qz[log_index] = q.z;
    history.qw[log_index] = q.w;
    history.t[log_index] = t;
    log_index++;
    // log_index = (log_index + 1) % Log_Entries;
}

void tick(Input io, float t, float dt)
{
    persist float euler_x = 0.0f;
    persist float euler_y = 0.0f;
    persist float euler_z = 0.0f;

    state.qd = m_quat_from_euler(euler_x, euler_y, euler_z);

    state.props[0].u = 2.5f;
    state.props[1].u = 2.5f;
    state.props[2].u = 2.5f;
    state.props[3].u = 2.5f;

    u32 integration_steps = 50;
    for (u32 i = 0; i < integration_steps; i++)
        ode_integrate(&state, dt/integration_steps);

    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 30.0f);
    // mat4 view = camera_holdclick(io, dt);
    mat4 view = mat_translate(0.0f, 0.0f, -5.0f) * mat_rotate_x(0.7f) * mat_rotate_y(0.0f);
    // mat4 model = mat_rotate_x(-PI/2.0f)*m_se3(m_quat_to_so3(state.q), m_vec3(0.0f))*mat_rotate_x(PI/2.0f)*mat_scale(0.5f);
    mat4 model = mat_rotate_x(-PI/2.0f)*m_se3(m_quat_to_so3(state.q), state.p_n)*mat_rotate_x(PI/2.0f)*mat_scale(0.5f);
    begin(&pass);
    depth_test(true, GL_LEQUAL);
    depth_write(true);
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clear(0.0f, 0.01f, 0.05f, 1.0f, 1.0f);
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

    add_history_entry(state.q, t);

    ImGui::NewFrame();
    ImGui::Begin("State");
    if (ImGui::CollapsingHeader("Desired orientation"))
    {
        ImGui::SliderAngle("ex", &euler_x);
        ImGui::SliderAngle("ey", &euler_y);
        ImGui::SliderAngle("ez", &euler_z);
    }
    ImGui::End();
    ImGui::Begin("Plots");
    ImGui::PlotLines("qx", history.qx, Log_Entries, 0, 0, -1.0f, +1.0f, ImVec2(0, 100));
    ImGui::PlotLines("qy", history.qy, Log_Entries, 0, 0, -1.0f, +1.0f, ImVec2(0, 100));
    ImGui::PlotLines("qz", history.qz, Log_Entries, 0, 0, -1.0f, +1.0f, ImVec2(0, 100));
    ImGui::PlotLines("qw", history.qw, Log_Entries, 0, 0, -1.0f, +1.0f, ImVec2(0, 100));
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
