#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

struct Robot
{
    vec2 position;
    float speed;
    float angle;
    float last_noise;
    float last_reverse;
};

#define NUM_TARGETS 10
#define NUM_OBSTACLES 4
#define NUM_REALIZATIONS 128
struct Realization
{
    Robot targets[NUM_TARGETS];
    Robot obstacles[NUM_OBSTACLES];
};

struct Hunter
{
    vec2 position;
    vec2 velocity;
    u32 target;
    float observe_radius;
} hunter;

Realization realizations[NUM_REALIZATIONS];

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

void draw_targets(Robot *targets)
{
    for (u32 i = 0; i < NUM_TARGETS; i++)
    {
        vec2 p = targets[i].position;
        float t = targets[i].angle;
        vec2 v = vec2(cos(t), sin(t)) * 0.3f;
        lines_add_line(p, p + v);
        vec2 dx = vec2(0.2f, 0.0f);
        vec2 dy = vec2(0.0f, 0.2f);
        lines_add_line(p - dx, p + dy);
        lines_add_line(p + dy, p + dx);
        lines_add_line(p + dx, p - dy);
        lines_add_line(p - dy, p - dx);
    }
}

void init_realizations()
{
    float dt = TWO_PI / NUM_TARGETS;
    for (u32 r = 0; r < NUM_REALIZATIONS; r++)
    {
        Robot *targets = realizations[r].targets;
        for (u32 i = 0; i < NUM_TARGETS; i++)
        {
            float t = i * dt;
            targets[i].position = vec2(cos(t), sin(t));
            targets[i].angle = t;
            targets[i].speed = 0.33f;
        }
    }
}

void init()
{
    lines_init();
    float a = (float)WINDOW_HEIGHT/WINDOW_WIDTH;
    lines_set_scale(a/12.0f, 1.0f/12.0f);
    lines_set_width(2.0f);
    init_realizations();
    hunter.position = vec2(0.0f, 0.0f);
    hunter.observe_radius = 0.5f;
}

void update_targets(Robot *targets, Input io, float t, float dt)
{
    for (u32 i = 0; i < NUM_TARGETS; i++)
    {
        Robot *target = targets + i;
        bool collision = false;
        for (u32 j = 0; j < NUM_TARGETS; j++)
        {
            if (i == j)
                continue;
            vec2 delta = targets[j].position - target->position;
            if (dot(delta, delta) <= 0.4f*0.4f)
            {
                collision = true;
                break;
            }
        }
        if (collision)
        {
            target->angle += PI;
        }
        if (t - target->last_reverse >= 20.0f)
        {
            target->last_reverse = t;
            target->angle += PI;
        }
        if (t - target->last_noise >= 5.0f)
        {
            target->last_noise = t;
            target->angle += (-1.0f + 2.0f * frand()) * 20.0f * PI / 180.0f;
        }
        if (target->angle >= TWO_PI) target->angle -= TWO_PI;
        if (target->angle < 0.0f) target->angle += TWO_PI;
        vec2 v = vec2(cos(target->angle), sin(target->angle)) * target->speed;
        target->position += v * dt;
    }
}

void update_hunter(Robot *mean_targets, Robot *true_targets, float t, float dt)
{
    persist float last_touch = 0.0f;
    vec2 dp = mean_targets[hunter.target].position - hunter.position;
    vec2 a = dp * 4.0f - hunter.velocity * 3.0f;
    hunter.velocity += a * dt;
    hunter.position += hunter.velocity * dt;
    hunter.observe_radius = 1.0f + 2.0f * smoothstepf(0.75f, 8.0f, length(hunter.velocity));

    if (t - last_touch > 5.0f)
    {
        last_touch = t;
        hunter.target = xor128() % NUM_TARGETS;
    }

    if (length(true_targets[hunter.target].position - hunter.position) < 0.25f &&
        hunter.observe_radius < 1.2f)
    {
        last_touch = t;
        true_targets[hunter.target].angle += PI / 4.0f;
        hunter.target = xor128() % NUM_TARGETS;
    }

    // Observe targets
    for (u32 i = 0; i < NUM_TARGETS; i++)
    {
        vec2 p = true_targets[i].position;
        if (length(hunter.position - p) <= hunter.observe_radius)
        {
            for (u32 r = 1; r < NUM_REALIZATIONS; r++)
                realizations[r].targets[i] = true_targets[i];
        }
    }
}

void tick(Input io, float t, float dt)
{
    persist bool draw_all_realizations = false;
    persist bool draw_covariance = false;
    persist bool draw_mean = false;

    Robot *true_targets = realizations[0].targets;
    for (u32 i = 0; i < NUM_REALIZATIONS; i++)
        update_targets(realizations[i].targets, io, t, dt);

    Robot mean_targets[NUM_TARGETS] = {};
    for (u32 i = 0; i < NUM_TARGETS; i++)
    {
        for (u32 r = 0; r < NUM_REALIZATIONS; r++)
        {
            mean_targets[i].position += realizations[r].targets[i].position;
            mean_targets[i].angle += realizations[r].targets[i].angle;
        }
        mean_targets[i].position *= 1.0f / (float)NUM_REALIZATIONS;
        mean_targets[i].angle *= 1.0f / (float)NUM_REALIZATIONS;
    }
    float cov_xx[NUM_TARGETS] = {};
    float cov_xy[NUM_TARGETS] = {};
    float cov_yy[NUM_TARGETS] = {};
    for (u32 i = 0; i < NUM_TARGETS; i++)
    {
        for (u32 r = 0; r < NUM_REALIZATIONS; r++)
        {
            vec2 delta = realizations[r].targets[i].position - mean_targets[i].position;
            cov_xx[i] += delta.x * delta.x;
            cov_xy[i] += delta.x * delta.y;
            cov_yy[i] += delta.x * delta.x;
        }
        cov_xx[i] /= (float)NUM_REALIZATIONS;
        cov_xy[i] /= (float)NUM_REALIZATIONS;
        cov_yy[i] /= (float)NUM_REALIZATIONS;
    }

    update_hunter(mean_targets, true_targets, t, dt);

    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clearc(0.03f, 0.02f, 0.01f, 1.0f);

    lines_set_color(vec4(1.0f, 0.95f, 0.7f, 0.15f));
    for (u32 i = 0; i <= 20; i++)
    {
        float x = (-1.0f + 2.0f * i / 20.0f) * 10.0f;
        lines_draw_line(x, -10.0f, x, +10.0f);
        lines_draw_line(-10.0f, x, +10.0f, x);
    }

    if (draw_covariance)
    {
        lines_set_color(vec4(0.2f, 0.2f, 1.0f, 1.0f));
        for (u32 i = 0; i < NUM_TARGETS; i++)
        {
            vec2 p = mean_targets[i].position;
            float dx = sqrt(cov_xx[i]);
            float dy = sqrt(cov_yy[i]);
            lines_add_line(p, p - vec2(dx, 0.0f));
            lines_add_line(p, p + vec2(dx, 0.0f));
            lines_add_line(p, p - vec2(0.0f, dy));
            lines_add_line(p, p + vec2(0.0f, dy));
        }
    }

    if (draw_all_realizations)
    {
        lines_set_color(vec4(1.0f, 0.95f, 0.7f, 0.05f));
        for (u32 i = 1; i < NUM_REALIZATIONS; i++)
            draw_targets(realizations[i].targets);
    }

    lines_set_color(vec4(1.0f, 1.0f, 1.0f, 0.35f));
    lines_draw_circle(hunter.position, hunter.observe_radius);

    if (draw_mean)
    {
        lines_set_color(vec4(1.0f, 0.95f, 0.7f, 0.35f));
        for (u32 i = 0; i < NUM_TARGETS; i++)
        {
            lines_add_line(mean_targets[i].position, true_targets[i].position);
        }
        lines_set_color(vec4(0.3f, 0.9f, 0.4f, 1.0f));
        draw_targets(mean_targets);
    }

    lines_set_color(vec4(1.0f, 0.3f, 0.1f, 1.0f));
    draw_targets(true_targets);

    lines_flush();

    ImGui::NewFrame();
    ImGui::Begin("Variables");
    ImGui::Checkbox("Draw all realizations", &draw_all_realizations);
    ImGui::Checkbox("Draw covariance", &draw_covariance);
    ImGui::Checkbox("Draw mean", &draw_mean);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
