/*
TODO:
Compute pair-intersection
Compute mean of pair-intersections
Compute the variance of each pair-intersection
Choose the pair-intersection with lowest variance as position estimate
Correct the remaining center positions from that

TODO:
Look at the change of a tower's center position estimate over time.
If it varies alot -> probably wrong -> weigh less in above computation.

TODO:
How to estimate a tower's position _along_ its circle?
Assume center position is correct -> correct position along circle?

TODO:
Machine learning for adaptive laws?
    Use machine learning / unsupervised learning to detect
    potential features to use as error driving terms in
    adaptive laws, by looking at time-series data.

TODO:
Probablilistic approach. Can include uncertainty, variance over time...
*/

#define USE_NEW_MATH
#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4

#define TOWER_RADIUS 5.0f
#define TOWER_SPEED 0.33f

struct Drone
{
    vec2 p;
    vec2 Dp;
};

struct Tower
{
    float t;
    vec2 c;
};

struct World
{
    Drone drone;
    Tower tower[4];
};

World world;
World filter;

void init()
{
    lines_init();
    lines_set_width(2.0f);

    world.drone.p = m_vec2(1.0f, 0.0f);
    world.drone.Dp = m_vec2(0.0f, 0.0f);

    filter.drone.p = m_vec2(1.0f, 0.0f);
    filter.drone.Dp = m_vec2(0.0f, 0.0f);

    // TODO: Determine initial angles and stuff
    filter.tower[0].t = 0.0f;
    filter.tower[1].t = PI / 2.0f;
    filter.tower[2].t = PI;
    filter.tower[3].t = 3.0f * PI / 2.0f;
    filter.tower[0].c = m_vec2(0.0f, 0.0f);
    filter.tower[1].c = m_vec2(0.0f, 0.0f);
    filter.tower[2].c = m_vec2(0.0f, 0.0f);
    filter.tower[3].c = m_vec2(0.0f, 0.0f);

    world.tower[0].t = 0.0f;
    world.tower[1].t = PI / 2.0f;
    world.tower[2].t = PI;
    world.tower[3].t = 3.0f * PI / 2.0f;
    world.tower[0].c = m_vec2(0.0f, 0.0f);
    world.tower[1].c = m_vec2(0.0f, 0.0f);
    world.tower[2].c = m_vec2(0.0f, 0.0f);
    world.tower[3].c = m_vec2(0.0f, 0.0f);
}

bool circle_intersect(vec2 c0, float r0,
                      vec2 c1, float r1,
                      vec2 hint, vec2 *out)
{
    float x0 = c0.x; float y0 = c0.y;
    float x1 = c1.x; float y1 = c1.y;
    float d = m_length(c1 - c0);
    if (d > r0+r1)
    {
        return false;
    }
    else if (d < m_abs(r0-r1))
    {
        return false;
    }
    else if (m_abs(d) <= 0.001f && m_abs(r0-r1) < 0.001f)
    {
        return false;
    }

    float a = (r0*r0-r1*r1+d*d) / (2.0f*d);
    float h = sqrt(r0*r0-a*a);
    vec2 pm = c0 + a*(c1-c0) / d;

    vec2 p1, p2;
    p1.x = pm.x + h*(y1-y0)/d;
    p1.y = pm.y - h*(x1-x0)/d;

    p2.x = pm.x - h*(y1-y0)/d;
    p2.y = pm.y + h*(x1-x0)/d;

    if (m_length(p2-hint) < m_length(p1-hint))
        *out = p2;
    else
        *out = p1;
    return true;
}

vec2 tower_pos(Tower tower)
{
    return tower.c + TOWER_RADIUS*m_vec2(cos(tower.t), sin(tower.t));
}

void tick(Input io, float t, float dt)
{
    // update towers
    {
        for (int i = 0; i < 4; i++)
        {
            float nt = (-1.0f + 2.0f * frand()) * 0.1f;
            float nx = (-1.0f + 2.0f * frand()) * 0.005f;
            float ny = (-1.0f + 2.0f * frand()) * 0.005f;
            world.tower[i].t += (1.0f + nt) * (TOWER_SPEED / TOWER_RADIUS) * dt;
            world.tower[i].c.x += nx;
            world.tower[i].c.y += ny;
        }
    }

    // control input
    vec2 DDp = m_vec2(0.0f, 0.0f);
    if (io_key_down(LEFT))  DDp.x -= 2.0f;
    if (io_key_down(RIGHT)) DDp.x += 2.0f;
    if (io_key_down(DOWN))  DDp.y -= 2.0f;
    if (io_key_down(UP))    DDp.y += 2.0f;
    if (m_length(world.drone.Dp) > 0.0f)
    {
        DDp -= m_normalize(world.drone.Dp) * 0.8f;
    }

    // update drone
    {
        world.drone.Dp += DDp * dt;
        world.drone.p += world.drone.Dp * dt;
    }

    // measurement accelerometer
    vec2 accelerometer;
    {
        float nx = (-1.0f + 2.0f * frand());
        float ny = (-1.0f + 2.0f * frand());
        float nxx = (-1.0f + 2.0f * frand());
        float nyy = (-1.0f + 2.0f * frand());
        vec2 n = sqrt(nx*nx+ny*ny) * (DDp + 0.1f*m_vec2(nxx, nyy)) / (0.8f+m_length(DDp));
        accelerometer = DDp + n;
    }

    // measurement lidar
    float lidar[4];
    {
        for (int i = 0; i < 4; i++)
        {
            float tt = world.tower[i].t;
            vec2 tc = world.tower[i].c;
            vec2 tp = tc + TOWER_RADIUS * m_vec2(cos(tt), sin(tt));
            lidar[i] = m_length(world.drone.p - tp);
        }
    }

    // update filter
    {
        for (int i = 0; i < 4; i++)
        {
            filter.tower[i].t += (TOWER_SPEED / TOWER_RADIUS) * dt;
        }
        filter.drone.Dp += accelerometer * dt;

        vec2 p1 = filter.drone.p + filter.drone.Dp * dt;
        // vec2 p3 = p1;
        // vec2 p4 = p1;
        // ...
        vec2 init = m_vec2(0.0f);
        vec2 estimate[] = {
            p1, p1, p1, p1,
            p1, p1, p1,
            p1, p1,
            p1
        };
        int index1s[] = {
            0, 0, 0, 0,
            1, 1, 1,
            2, 2,
            3
        };
        int index2s[] = {
            0, 1, 2, 3,
            1, 2, 3,
            2, 3,
            3
        };
        vec2 sum = m_vec2(0.0f);
        for (int ci = 0; ci < 10; ci++)
        {
            int i1 = index1s[ci];
            int i2 = index2s[ci];
            vec2 c1 = tower_pos(filter.tower[i1]);
            vec2 c2 = tower_pos(filter.tower[i2]);
            float r1 = lidar[i1];
            float r2 = lidar[i2];
            circle_intersect(c1, r1, c2, r2, filter.drone.p, &estimate[ci]);
            sum += estimate[ci];
        }
        sum /= 10.0f;

        filter.drone.p = 0.3f * p1 + 0.7f * sum;

        // next, now that we have a good confidence estimate of our own position,
        // correct the tower robot centers
        for (int ti = 0; ti < 4; ti++)
        {
            float r = lidar[ti];
            vec2 px = filter.drone.p;
            vec2 p1 = tower_pos(filter.tower[ti]);
            vec2 p2 = px + m_normalize(p1 - px) * r; // closest point on circle
            vec2 dp = p2 - p1;
            // TODO: This needs improvement
            filter.tower[ti].c += 5.0f*dp*dt;
        }
    }

    blend_mode(true);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    lines_set_scale((WINDOW_HEIGHT / (r32)WINDOW_WIDTH)/10.0f, 1.0f/10.0f);

    lines_set_color(0xB23F24FF);
    lines_draw_circle(world.drone.p, 0.5f);
    lines_set_color(0x78B041FF);
    lines_draw_circle(filter.drone.p, 0.5f);

    lines_set_color(0x3490E0FF);
    for (int i = 0; i < 4; i++)
    {
        vec2 tc = world.tower[i].c;
        vec2 tp = tower_pos(world.tower[i]);
        lines_draw_circle(tc, 0.1f);
        lines_draw_line(tc, tp);
        lines_draw_circle(tp, 0.5f);
    }

    lines_set_color(0xB23F2477);
    for (int i = 0; i < 4; i++)
    {
        lines_draw_circle(filter.tower[i].c, 0.2f);
        lines_draw_circle(tower_pos(filter.tower[i]), lidar[i], 64);
        // lines_draw_line(tower_pos(filter.tower[i]), world.drone.p);
    }

    lines_flush();
}

#include "sumo.cpp"
