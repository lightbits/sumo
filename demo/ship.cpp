#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 700
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

void lines_draw_circle(vec2 p, float r)
{
    vec2 points[32] = {};
    for (u32 i = 0; i < 32; i++)
    {
        float a = TWO_PI * i / 31.0f;
        vec2 d = vec2(cos(a), sin(a)) * r;
        points[i] = p + d;
    }
    lines_draw_poly(points, 32);
}

void init()
{
    lines_init();
}

void imgui_float(char *label, float x)
{
    char text[256];
    sprintf(text, "%s = %.2f", label, x);
    ImGui::Text(text);
}

persist float xt = 100.0f;
persist float yt = 100.0f;
persist vec2 vt = normalize(vec2(1.0f, 1.0f)) * 3.0f;

void path_guidance(float t, float x, float y, float *xd, float *yd)
{
    // *xd = 5000.0f;
    // *yd = 5000.0f;

    vec2 pt = vec2(xt, yt);
    vec2 n = normalize(vec2(vt.x, -vt.y));
    vec2 pd = pt + n * 500.0f;

    *xd = pd.x;
    *yd = pd.y;

    // persist u32 i = 0;
    // const u32 N = 5;
    // vec2 WP[N] = {
    //     vec2(-600.0f, 0.0f),
    //     vec2(4680.0f, 3880.0f),
    //     vec2(7662.0f, 2248.0f),
    //     vec2(10644.0f, 3878.0f),
    //     vec2(13626.0f, 2246.0f)
    // };
    // vec2 a = WP[i];
    // vec2 b = WP[i+1];
    // vec2 p = vec2(x, y);
    // float R = 1250;
    // vec2 r = normalize(b-a);
    // float B = dot(p-a, r);
    // float C = dot(p-a,p-a) - R*R;
    // float D = B*B-C;
    // if (D < 0)
    // {
    //     float s = dot(p-a, r);
    //     *xd = a.x + r.x*s;
    //     *yd = a.y + r.y*s;
    // }
    // else
    // {
    //     float sD = sqrt(D);
    //     float s = max(B+sD, B-sD);
    //     *xd = a.x + r.x*s;
    //     *yd = a.y + r.y*s;
    // }
    // if (length(b-p) < R && i < N-2)
    //     i++;
}

float heading_guidance(float x, float y, float xd, float yd, float u, float v)
{
    vec2 pd = vec2(xd, yd);
    vec2 p = vec2(x, y);

    vec2 dp = pd - p;
    float D = 1000.0f;
    float K = 3.0f;
    float f = clampf(length(dp), 0.0f, D);
    vec2 ve = vec2(0.0f, 0.0f);
    if (f > D / 100000.0f)
    {
        ve = (dp / length(dp)) * f * K / D;
    }
    vec2 vd = vt + ve;

    float khi_d = atan2(vd.y, vd.x);
    float beta = atan2(v, u);
    float psi_d = khi_d - beta;
    return psi_d;
}

float surge_guidance(float x, float y, float xd, float yd)
{
    vec2 pd = vec2(xd, yd);
    vec2 p = vec2(x, y);

    vec2 dp = pd - p;
    float D = 1000.0f;
    float K = 3.0f;
    float f = clampf(length(dp), 0.0f, D);
    vec2 ve = vec2(0.0f, 0.0f);
    if (f > D / 100000.0f)
    {
        ve = (dp / length(dp)) * f * K / D;
    }
    vec2 vd = vt + ve;

    return length(vd);
}

float surge_controller(float u_d0, float u, float dt)
{
    persist float ei = 0.0f;
    persist float u_d = 0.0f;
    persist float Du_d = 0.0f;
    float T_surge = 100.0f;

    Du_d = (u_d0 - u_d) / T_surge;
    u_d += Du_d * dt;

    float m = 9000000;
    float d1 = 80;
    float d2 = 1500;
    float k = 1500;
    float kp = 0.03;
    float ki = kp*kp/4;

    float e = u-u_d;
    float Dei = e;
    ei += Dei * dt;

    float tau = u_d * (d1 + d2*abs(u)) + m * (Du_d - kp*e - ki*ei);

    if (abs(tau) > 1.0f)
        return tau / sqrt(abs(tau)*k);
    else
        return 0.0f;
}

float heading_controller(float psi_d0, float psi, float r, float dt)
{
    persist float psi_d = 0.0f;
    persist float r_d = 0.0f;
    persist float a_d = 0.0f;
    persist float ei = 0.0f;
    float T_heading = 80.0f;

    a_d = (psi_d0 - psi_d - 2.0f*T_heading*r_d) / (T_heading*T_heading);
    r_d += a_d * dt;
    psi_d += r_d * dt;

    float Dei = psi-psi_d;
    ei += Dei * dt;

    float T = 36;
    float k = -0.018;
    float wd = 0.2;
    float kd = wd*2*T;
    float kp = kd*kd/(4*T);
    float ki = wd/10;
    return (-kp*(psi-psi_d)-kd*(r-r_d) + r + T*a_d - ki*ei)/k;
}

void tick(Input io, float t, float dt)
{
    clearc(0.0f, 0.01f, 0.02f, 1.0f);

    struct Ship
    {
        float x;
        float y;
        float u;
        float v;
        float r;
        float psi;
        float beta;
    };

    persist Ship ship;
    persist bool loaded = false;
    persist float sim_time = 0.0f;
    if (!loaded)
    {
        ship.x = 0.0f;
        ship.y = 0.0f;
        ship.u = 6.63f;
        ship.v = 0.0f;
        ship.r = 0.0f;
        ship.psi = 0.0f;
        loaded = true;
    }

    u32 sim_steps = 1000;
    float timestep = 500.0f*dt / (float)sim_steps;
    float xd, yd;
    for (u32 i = 0; i < sim_steps; i++)
    {
        ship.v = 1.0f;
        sim_time += timestep;
        path_guidance(sim_time, ship.x, ship.y, &xd, &yd);

        xt += vt.x * timestep;
        yt += vt.y * timestep;

        ship.beta = atan2(ship.v, ship.u);

        // integrate ship: heading
        {
            float psi_d0 = heading_guidance(ship.x, ship.y, xd, yd, ship.u, ship.v);
            float dc = heading_controller(psi_d0, ship.psi, ship.r, timestep);

            float T = 36;
            float k = -0.018;
            float Dpsi = ship.r;
            float Dr = (-ship.r + k * dc)/T;
            ship.r += Dr * timestep;
            ship.psi += Dpsi * timestep;
        }

        // integrate ship: surge
        {
            float u_d0 = surge_guidance(ship.x, ship.y, xd, yd);
            float nc = surge_controller(u_d0, ship.u, timestep);

            float m = 9000000;
            float d1 = 80;
            float d2 = 1500;
            float k = 1500;
            float Du = (k*nc*abs(nc) - d1*ship.u - d2*abs(ship.u)*ship.u)/m;
            ship.u += Du * timestep;
        }

        // integrate ship: position
        {
            float c = cos(ship.psi + ship.beta);
            float s = sin(ship.psi + ship.beta);
            ship.x += ship.u * c * timestep;
            ship.y += ship.u * s * timestep;
        }
    }

    #define Log_Entries 256
    struct History
    {
        vec2 p[Log_Entries];
        float t[Log_Entries];
    };

    persist History history;
    persist u32 log_index = 0;

    vec2 p = vec2(ship.x, ship.y);
    vec2 vn = vec2(ship.u * cos(ship.psi) - ship.v * sin(ship.psi),
                   ship.u * sin(ship.psi) + ship.v * cos(ship.psi));

    history.p[log_index] = p;
    history.t[log_index] = sim_time;

    lines_set_scale(1.0f / 15000.0f, 1.0f / 15000.0f);
    lines_set_width(2.0f);
    lines_set_color(vec4(1.0f, 0.5f, 0.1f, 1.0f));
    lines_draw_circle(p, 50.0f);
    lines_draw_line(p, p + normalize(vn) * 1000.0f);

    vec2 pd = vec2(xd, yd);
    lines_set_color(vec4(0.3f, 0.9f, 0.7f, 1.0f));
    lines_draw_circle(pd, 50.0f);

    lines_set_color(vec4(1.0f, 0.2f, 0.1f, 1.0f));
    lines_draw_circle(vec2(xt, yt), 50.0f);

    lines_set_color(vec4(0.3f, 0.9f, 0.7f, 1.0f));
    for (u32 i = 0; i < Log_Entries-1; i++)
    {
        if (history.t[i+1] > history.t[i])
            lines_draw_line(history.p[i], history.p[i+1]);
    }

    log_index = (log_index + 1) % Log_Entries;

    lines_flush();
}

#include "sumo.cpp"
