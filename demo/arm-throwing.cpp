#include "sumo.h"
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 700
#define MULTISAMPLES 4

static r32 q1;
static r32 q2;
static r32 Dq1;
static r32 Dq2;
static r32 x1;
static r32 x2;
static r32 Dx1;
static r32 Dx2;

static r32 l1 = 0.3f;
static r32 l2 = 0.542f;
static r32 m1 = 2.934f;
static r32 m2 = 1.1022f;
static r32 lc1 = 0.2071f;
static r32 lc2 = 0.2717f;
static r32 I1 = 0.2067f;
static r32 I2 = 0.1362f;
static r32 k2 = 14.1543f;
static r32 g = 9.81f;
static r32 mb = 0.064f;

static int mode;
static r32 t = 0.0f;

static r32 f_sequence[8];
static int f_sequence_i;

static r32 distance;
static r32 scale = 1.0f;
static bool power_limit = false;

r32 x1_of_q(r32 q1, r32 q2)
{
    return l1*cos(q1)+l2*cos(q1+q2);
}

r32 x2_of_q(r32 q1, r32 q2)
{
    return l1*sin(q1)+l2*sin(q1+q2);
}

void sim_reset()
{
    q1 = 5.0f*PI/6.0f;
    q2 = PI + asin(l1/(2.0f*l2)) - q1;
    Dq1 = 0.0f;
    Dq2 = 0.0f;
    x1 = x1_of_q(q1, q2);
    x2 = x2_of_q(q1, q2);
    Dx1 = 0.0f;
    Dx2 = 0.0f;
    mode = 1;
    f_sequence_i = 0;
    t = 0.0f;
    distance = 0.0f;
    scale = 1.0f;
    power_limit = false;
}

#include <stdio.h>

void sim_step(int steps, r32 dt)
{
    for (int i = 0; i < steps; i++)
    {
        t += dt;
        r32 c1 = cos(q1);
        r32 s1 = sin(q1);
        r32 c2 = cos(q2);
        r32 s2 = sin(q2);
        r32 c12 = cos(q1+q2);
        r32 s12 = sin(q1+q2);

        if (mode == 1)
        {
            x1 = l1*c1+l2*c12;
            x2 = l1*s1+l2*s12;
            Dx1 = -Dq1*l1*s1-(Dq1+Dq2)*l2*s12;
            Dx2 = Dq1*l1*c1+(Dq1+Dq2)*l2*c12;
        }

        if (x1 >= 0.0f && mode == 1)
        {
            mode = 2;
            distance = Dx1*(Dx2+sqrt(Dx2*Dx2+2.0f*g*x2))/g;
        }

        r32 F1 = 0.0f;
        r32 F2 = 0.0f;
        if (mode == 1)
        {
            f_sequence_i = (int)(array_count(f_sequence)*(t / 2.0f));
            if (f_sequence_i < array_count(f_sequence))
            {
                F1 = f_sequence[f_sequence_i];
                // F1 = 1.6f-60.0f*t-5.0f*t*t;
            }
            else
            {
                F1 = 0.0f;
            }
        }
        else
        {
            r32 DDx1 = 0.0f;
            r32 DDx2 = -g;
            Dx1 += dt*DDx1;
            Dx2 += dt*DDx2;
            x1 += dt*Dx1;
            x2 += dt*Dx2;
            F1 = -4.0f*Dq1;

            if (F1 > 180.0f) F1 = 180.0f;
            if (F1 < -180.0f) F1 = -180.0f;
            if (abs(F1*Dq1) > 270.0f)
            {
                power_limit = true;
            }
            else
                power_limit = false;
        }

        r32 denom = I1*I2 + l1*l1*lc2*lc2*m2*m2 + I2*l1*l1*m2 + I2*lc1*lc1*m1 + I1*lc2*lc2*m2 + I2*lc2*lc2*m2 - c2*c2*l1*l1*lc2*lc2*m2*m2 + lc1*lc1*lc2*lc2*m1*m2 + 2*I2*c2*l1*lc2*m2;
        r32 DDq1 = (F1*I2 + F1*lc2*lc2*m2 + k2*lc2*lc2*m2*q2 - c1*g*l1*lc2*lc2*m2*m2 - I2*c1*g*l1*m2 - I2*c1*g*lc1*m1 - I2*c12*g*lc2*m2 + Dq1*Dq1*l1*lc2*lc2*lc2*m2*m2*s2 + Dq2*Dq2*l1*lc2*lc2*lc2*m2*m2*s2 + Dq2*Dq2*I2*l1*lc2*m2*s2 - c1*g*lc1*lc2*lc2*m1*m2 + Dq1*Dq1*c2*l1*l1*lc2*lc2*m2*m2*s2 + 2*Dq1*Dq2*l1*lc2*lc2*lc2*m2*m2*s2 + c2*c12*g*l1*lc2*lc2*m2*m2 + c2*k2*l1*lc2*m2*q2 + 2*Dq1*Dq2*I2*l1*lc2*m2*s2)/denom;
        r32 DDq2 = -(I1*k2*q2 + F1*lc2*lc2*m2 + k2*l1*l1*m2*q2 + k2*lc1*lc1*m1*q2 + k2*lc2*lc2*m2*q2 - c1*g*l1*lc2*lc2*m2*m2 + c12*g*l1*l1*lc2*m2*m2 + I1*c12*g*lc2*m2 + F1*c2*l1*lc2*m2 + Dq1*Dq1*l1*lc2*lc2*lc2*m2*m2*s2 + Dq1*Dq1*l1*l1*l1*lc2*m2*m2*s2 + Dq2*Dq2*l1*lc2*lc2*lc2*m2*m2*s2 + Dq1*Dq1*I1*l1*lc2*m2*s2 - c1*g*lc1*lc2*lc2*m1*m2 + c12*g*lc1*lc1*lc2*m1*m2 + 2*Dq1*Dq1*c2*l1*l1*lc2*lc2*m2*m2*s2 + Dq2*Dq2*c2*l1*l1*lc2*lc2*m2*m2*s2 + 2*Dq1*Dq2*l1*lc2*lc2*lc2*m2*m2*s2 - c1*c2*g*l1*l1*lc2*m2*m2 + c2*c12*g*l1*lc2*lc2*m2*m2 + 2*c2*k2*l1*lc2*m2*q2 + 2*Dq1*Dq2*c2*l1*l1*lc2*lc2*m2*m2*s2 + Dq1*Dq1*l1*lc1*lc1*lc2*m1*m2*s2 - c1*c2*g*l1*lc1*lc2*m1*m2)/denom;

        Dq1 += dt*DDq1;
        Dq2 += dt*DDq2;
        q1 += dt*Dq1;
        q2 += dt*Dq2;
    }
}

void init()
{
    sim_reset();
    lines_init();
}

void tick(Input io, float app_t, float app_dt)
{
    clearc(1.0f, 1.0f, 1.0f, 1.0f);
    // -20.560
    // +21.682
    // -39.999
    // -180
    // => 40 meters!

    // +16.847
    // -110
    // +19.9

    #if 1
    persist r32 sim_time = 1.0f;
    int sim_steps = 2000;
    r32 sim_dt = sim_time / sim_steps;
    persist r32 sim_dt_multiplier = 0.993f;

    blend_mode(true);
    lines_set_width(1.0f);
    lines_set_scale(0.5f, 0.5f);
    sim_reset();
    for (int i = 0; i < sim_steps; i+=10)
    {
        sim_dt *= sim_dt_multiplier;
        sim_step(10, sim_dt);
        if (i + 10 >= sim_steps)
        {
            lines_set_color(0xff3322ff);
        }
        else
        {
            lines_set_color(0x00000044);
        }
        lines_draw_line(0.0f, 0.0f,
                        l1*cos(q1), l1*sin(q1));
        lines_draw_line(l1*cos(q1), l1*sin(q1),
                        l1*cos(q1)+l2*cos(q1+q2), l1*sin(q1)+l2*sin(q1+q2));
        lines_draw_circle(x1, x2, 0.05f);

        if (mode == 2)
        {
            lines_set_color(0xff5522ff);
            lines_draw_line(x1, x2, x1+Dx1, x2+Dx2);
        }
    }

    lines_flush();

    ImGui::NewFrame();
    ImGui::Text("%.4f", distance);
    ImGui::Text("%d", f_sequence_i);
    ImGui::SliderFloat("dtmul", &sim_dt_multiplier, 0.9f, 1.0f);
    ImGui::SliderFloat("time", &sim_time, 0.1f, 2.0f);
    {
        for (int i = 0; i < array_count(f_sequence); i++)
        {
            char label[256];
            sprintf(label, "f%d", i);
            ImGui::SliderFloat(label, &f_sequence[i], -180.0f, 180.0f);
        }
    }
    ImGui::Render();
    #else

    // f_sequence[0] = -20.560;
    // f_sequence[1] = +21.682;
    // f_sequence[2] = -39.999;
    // f_sequence[3] = -180;

    f_sequence[0] = +16.847f;
    f_sequence[1] = -110.0f;
    f_sequence[2] = +19.9f;

    r32 dt = app_dt / 1000.0f;
    persist r32 speed = 1.0f;
    sim_step((int)(1000*speed), dt);

    lines_set_width(3.0f);
    lines_set_scale(0.4f, 0.4f);
    lines_set_color(0x5577ffff);
    lines_draw_line(0.0f, 0.0f,
                    l1*cos(q1), l1*sin(q1));
    lines_draw_line(l1*cos(q1), l1*sin(q1),
                    l1*cos(q1)+l2*cos(q1+q2), l1*sin(q1)+l2*sin(q1+q2));
    lines_draw_circle(x1, x2, 0.05f);
    lines_set_color(0x000000ff);
    lines_draw_line(0.0f, 0.0f, 100.0f, 0.0f);
    lines_draw_line(0.0f, 0.0f, 0.0f, 0.4f);
    lines_flush();
    ImGui::NewFrame();
    ImGui::SliderFloat("speed", &speed, 0.1f, 10.0f);
    ImGui::Text("DIST = %.1fm", distance);
    ImGui::Text(power_limit ? "POWER LIMIT REACHED" : "POWER LIMIT OK");
    ImGui::Render();
    #endif

}

#include "sumo.cpp"
