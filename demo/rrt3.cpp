#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

#define TREE_NODES (4096*2)
struct Tree
{
    r32 x1[TREE_NODES];
    r32 x2[TREE_NODES];
    r32 parent_u[TREE_NODES];
    s32 parent_i[TREE_NODES];
    s32 next_i[TREE_NODES];
    s32 count;

    s32 best;
} tree;

r32 l = 0.5f;
r32 m = 1.0f;
r32 I = m*l*l;
r32 g = 9.81f;
r32 b = 0.2f;

r32 DX1(r32 x1, r32 x2, r32 u)
{
    return x2;
}

r32 DX2(r32 x1, r32 x2, r32 u)
{
    return (u - m*g*l*sin(x1) - l*b*x2) / I;
}

void init()
{
    tree.x1[0] = 0.0f;
    tree.x2[0] = 0.0f;
    tree.count = 1;
    tree.best = 0;
    lines_init();
}
#include <stdio.h>
void tick(Input io, float t, float dt)
{
    r32 target_x1 = TWO_PI;
    r32 target_x2 = 0.0f;
    for (u32 it = 0; it < 64; it++)
    if (tree.count < TREE_NODES)
    {
        // select random point in state-space
        r32 x1, x2;
        if (frand() < 0.05f)
        {
            x1 = target_x1;
            x2 = target_x2;
        }
        else
        {
            x1 = (-1.0f + 2.0f * frand()) * TWO_PI;
            x2 = (-1.0f + 2.0f * frand()) * 1.5f * TWO_PI;
        }

        #define DISTANCE(ax1, ax2, bx1, bx2) m_length(m_vec2(ax1, ax2) - m_vec2(bx1, bx2))

        // find closest node in tree
        u32 closestn = 0;
        r32 closestd = DISTANCE(x1, x2, tree.x1[0], tree.x2[0]);
        for (u32 n = 1; n < tree.count; n++)
        {
            r32 d = DISTANCE(x1, x2, tree.x1[n], tree.x2[n]);
            if (d < closestd)
            {
                closestd = d;
                closestn = n;
            }
        }

        // try various control inputs and select the one
        // that gets us closest to the new point
        r32 x10 = tree.x1[closestn];
        r32 x20 = tree.x2[closestn];
        r32 dt = 0.01f;
        r32 closestuu = 0.0f;
        r32 closestud = DISTANCE(x10, x20, x1, x2);
        for (u32 i = 0; i < 16; i++)
        {
            r32 u = -1.0f + 2.0f * frand();
            r32 x11 = x10 + DX1(x10, x20, u) * dt;
            r32 x21 = x20 + DX2(x10, x20, u) * dt;
            r32 d = DISTANCE(x11, x21, x1, x2);
            if (d < closestud)
            {
                closestud = d;
                closestuu = u;
            }
        }

        s32 i = tree.count;
        tree.x1[i] = x10 + DX1(x10, x20, closestuu) * dt;
        tree.x2[i] = x20 + DX2(x10, x20, closestuu) * dt;
        tree.parent_i[i] = closestn;
        tree.parent_u[i] = closestuu;

        if (DISTANCE(tree.x1[i], tree.x2[i], target_x1, target_x2) <
            DISTANCE(tree.x1[tree.best], tree.x2[tree.best], target_x1, target_x2))
        {
            tree.best = i;
        }

        tree.count++;
    }

    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    blend_mode(true);
    r32 aspect = WINDOW_HEIGHT / (r32)WINDOW_WIDTH;
    r32 range = 2.0f*TWO_PI;
    lines_set_width(1.0f);
    lines_set_scale(aspect / range, 1.0f / range);
    lines_set_color(m_vec4(1.0f, 1.0f, 1.0f, 0.2f));
    for (s32 i = 1; i < tree.count; i++)
    {
        r32 x10 = tree.x1[tree.parent_i[i]];
        r32 x20 = tree.x2[tree.parent_i[i]];
        r32 x11 = tree.x1[i];
        r32 x21 = tree.x2[i];
        lines_draw_line(x10, x20, x11, x21);
    }
    lines_set_color(m_vec4(1.0f, 0.5f, 0.5f, 1.0f));
    lines_draw_circle(TWO_PI, 0.0f, 0.1f);

    lines_set_width(2.0f);
    lines_set_color(m_vec4(1.0f, 0.1f, 0.1f, 1.0f));
    s32 i = tree.best;
    s32 link_length = 0;
    while (i > 0)
    {
        r32 x10 = tree.x1[tree.parent_i[i]];
        r32 x20 = tree.x2[tree.parent_i[i]];
        r32 x11 = tree.x1[i];
        r32 x21 = tree.x2[i];
        lines_draw_line(x10, x20, x11, x21);
        i = tree.parent_i[i];
        link_length++;
    }
    link_length++;

    lines_flush();

    persist r32 x1 = 0.0f;
    persist r32 x2 = 0.0f;
    persist s32 u_count = 0;
    r32 sim_dt = 0.001f;
    if (io_key_down(SPACE))
    {
        s32 i = tree.best;
        r32 u = 0.0f;
        for (s32 j = 0; j < u_count; j++)
        {
            u = tree.parent_u[i];
            i = tree.parent_i[i];
        }

        u_count--;

        for (u32 k = 0; k < 10; k++)
        {
            r32 dx1 = DX1(x1, x2, u);
            r32 dx2 = DX2(x1, x2, u);
            x1 += dx1*sim_dt;
            x2 += dx2*sim_dt;
        }
    }
    else
    {
        u_count = link_length;
    }

    lines_set_scale(aspect, 1.0f);
    lines_draw_line(0.0f, 0.0f, l*sin(x1), -l*cos(x1));
    lines_flush();
}

#include "sumo.cpp"
