#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define SHADER(src) "#version 150\n" #src

char *vs = SHADER(
in vec2 position;
uniform float aspect;
out vec2 v_position;
void main()
{
    v_position = position * vec2(aspect, 1.0);
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
);

char *fs = SHADER(
in vec2 v_position;
uniform float time;
out vec4 f_color;
const float pi = 3.1415926;
float shuriken(vec2 p, float r1, float r2)
{
    return length(p) - (r1 + r2*sin(10.0*atan(p.y, p.x)));
}

float box(vec2 p, float w, float h)
{
    return max(abs(p.x), abs((w/h)*p.y))-w;
}

float circle(vec2 p, float r)
{
    return sqrt(p.x*p.x+p.y*p.y) - r;
}

vec2 rotate(vec2 p, float t)
{
    float c = cos(t);
    float s = sin(t);
    return vec2(p.x*c-p.y*s,
                p.x*s+p.y*c);
}

float map(vec2 p)
{
    // float d1 = box(rotate(p, -pi/4.0), 0.25, 0.05);
    // float d2 = box(rotate(p, +pi/4.0), 0.25, 0.05);
    // float d3 = shuriken(p, 0.5, 0.1);
    // float d4 = box(p - vec2(0.75, 0.0), 0.5, 0.05);
    // float d5 = box(p - vec2(-0.75, 0.0), 0.5, 0.05);
    // return min(min(d4, d5), max(d3, -min(d1, d2)));

    float d1 = box(rotate(p, 0.5), 0.5, 0.25);
    float d2 = box(p - vec2(-0.5, 0.0), 0.6, 0.1);
    float d3 = circle(p - sin(time)*vec2(0.5, -0.5), 0.2);
    float d4 = box(rotate(p, -0.5) - vec2(0.3, 0.2), 0.6, 0.1);
    return max(min(d1, min(d2, d4)), -d3);
}

void main()
{
    float d = map(v_position);
    // float t = 1.0 - smoothstep(0.0, 0.003, d);
    // f_color = mix(vec4(0.0), vec4(0.8, 0.2, 0.2, 1.0), t);
    if (d < 0.0)
        f_color = vec4(0.8, 0.2, 0.2, 1.0);
    else
        f_color = vec4(0.0);
}
);

RenderPass pass;

#define TREE_NODES 1024
struct Tree
{
    r32 x[TREE_NODES];
    r32 y[TREE_NODES];
    s32 parent[TREE_NODES];
    s32 count;
} tree;

float box(vec2 p, float w, float h)
{
    return m_max(m_abs(p.x), m_abs((w/h)*p.y))-w;
}

float circle(vec2 p, float r)
{
    return sqrt(p.x*p.x+p.y*p.y) - r;
}

vec2 rotate(vec2 p, float t)
{
    float c = cos(t);
    float s = sin(t);
    return m_vec2(p.x*c-p.y*s,
                p.x*s+p.y*c);
}

float map(float time, vec2 p)
{
    float d1 = box(rotate(p, 0.5), 0.5, 0.25);
    float d2 = box(p - m_vec2(-0.5, 0.0), 0.6, 0.1);
    float d3 = circle(p - sin(time)*m_vec2(0.5, -0.5), 0.2);
    float d4 = box(rotate(p, -0.5) - m_vec2(0.3, 0.2), 0.6, 0.1);
    return m_max(m_min(d1, m_min(d2, d4)), -d3);
}

float distance(u32 i, r32 x, r32 y)
{
    r32 dx = x-tree.x[i];
    r32 dy = y-tree.y[i];
    return sqrt(dx*dx+dy*dy);
}

void init()
{
    pass = make_render_pass(vs, fs);
    tree.x[0] = -1.0f;
    tree.y[0] = 0.0f;
    tree.count = 1;

    lines_init();
}

void tick(Input io, float time, float dt)
{
    time = 0.0f;
    persist r32 reset_timer = 0.3f;
    persist r32 target_x = io.mouse.ndc.x * WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    persist r32 target_y = io.mouse.ndc.y;
    reset_timer -= dt;
    if (reset_timer < 0.0f)
    {
        reset_timer = 0.3f;
        tree.count = 1;
        target_x = io.mouse.ndc.x * WINDOW_WIDTH / (float)WINDOW_HEIGHT;
        target_y = io.mouse.ndc.y;

        while (tree.count < TREE_NODES)
        {
            r32 x, y;
            if (frand() < 0.1f)
            {
                x = target_x;
                y = target_y;
            }
            else
            {
                // select random point in state-space
                x = -1.0f + 2.0f * frand();
                y = -1.0f + 2.0f * frand();
            }

            // find closest feasible tree node
            u32 closestn = 0;
            r32 closestd = distance(0, x, y);
            for (u32 n = 1; n < tree.count; n++)
            {
                r32 d = distance(n, x, y);
                if (d < closestd)
                {
                    closestd = d;
                    closestn = n;
                }
            }

            int feasible = 0;
            if (map(time, m_vec2(x, y)) >= 0.0f)
            {
                feasible = 0;
            }
            else
            {
                // determine if the path towards the point
                // is feasible by raymarching
                r32 t = 0.0f;
                u32 max_steps = 16;
                vec2 p0 = m_vec2(tree.x[closestn], tree.y[closestn]);
                vec2 rd = m_normalize(m_vec2(x, y) - p0);
                for (u32 i = 0; i < max_steps; i++)
                {
                    if (t >= closestd)
                    {
                        feasible = 1;
                        break;
                    }
                    r32 d = -map(time, p0 + t * rd);
                    if (d <= 0.0f)
                        break;
                    t += d;
                }
            }

            if (!feasible)
            {
                continue;
            }

            u32 write_index = tree.count;
            tree.x[write_index] = x;
            tree.y[write_index] = y;
            tree.parent[write_index] = closestn;
            tree.count++;
            if (distance(write_index, target_x, target_y) < 0.01f)
                break;
        }
    }

    lines_set_scale(WINDOW_HEIGHT / (float)WINDOW_WIDTH, 1.0f);
    begin(&pass);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    uniformf("time", time);
    uniformf("aspect", WINDOW_WIDTH / (float)WINDOW_HEIGHT);
    so_draw_fullscreen_quad();

    blend_mode(true);
    for (u32 i = 1; i < tree.count; i++)
    {
        lines_set_color(m_vec4(1.0f, 0.9f, 0.9f, 0.5f));
        r32 x0 = tree.x[tree.parent[i]];
        r32 y0 = tree.y[tree.parent[i]];
        r32 x1 = tree.x[i];
        r32 y1 = tree.y[i];
        lines_draw_line(x0, y0, x1, y1);
    }

    if (distance(tree.count-1, target_x, target_y) < 0.01f)
    {
        lines_set_color(m_vec4(0.2f, 1.0f, 0.2f, 1.0f));
        u32 i = tree.count-1;
        u32 parent = tree.parent[i];
        while (parent != 0)
        {
            r32 x0 = tree.x[parent];
            r32 y0 = tree.y[parent];
            r32 x1 = tree.x[i];
            r32 y1 = tree.y[i];
            lines_draw_line(x0, y0, x1, y1);
            i = parent;
            parent = tree.parent[i];
        }
    }
    lines_flush();
}

#include "sumo.cpp"
