#include "sumo.h"
#include <stdio.h>
#include "fsm.cpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
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

struct GroundRobotModel
{
    float x;
    float y;
    float L;
    float vl;
    float vr;
    float q;
};

struct GroundRobot
{
    fsm_RobotState state;
    fsm_RobotInternalState internal;
    fsm_Action action;
    GroundRobotModel model;
};

#define Num_Targets 10
#define Num_Obstacles 4
#define Num_Robots (Num_Targets + Num_Obstacles)
static GroundRobot robots[Num_Robots];
static GroundRobot *targets;
static GroundRobot *obstacles;

void init()
{
    lines_init();
    lines_set_width(2.0f);

    targets = robots;
    obstacles = robots + Num_Targets;

    for (u32 i = 0; i < Num_Targets; i++)
    {
        float t = TWO_PI * i / (float)(Num_Targets);

        GroundRobot robot = {};
        robot.model.L = 0.5f;
        robot.model.x = -1.0f * sin(t);
        robot.model.y = 1.0f * cos(t);
        robot.model.q = t;
        robot.internal.initialized = false;
        robot.state = Robot_Start;

        targets[i] = robot;
    }

    for (u32 i = 0; i < Num_Obstacles; i++)
    {
        float t = TWO_PI * i / (float)(Num_Obstacles);

        GroundRobot robot = {};
        robot.model.L = 0.5f;
        robot.model.x = -5.0f * sin(t);
        robot.model.y = 5.0f * cos(t);
        robot.model.q = t + PI / 2.0f;
        robot.internal.initialized = false;
        robot.state = Robot_Start;

        obstacles[i] = robot;
    }
}

void integrate_robot(GroundRobotModel *robot, float dt)
{
    float v = 0.5f * (robot->vl + robot->vr);
    float w = (robot->vr - robot->vl) / (robot->L*0.5f);
    robot->q += w * dt;
    robot->x += -v * sin(robot->q) * dt;
    robot->y +=  v * cos(robot->q) * dt;
}

void tick_simulator(float t, float dt)
{
    struct CollisionInfo
    {
        int hits;
        vec2 resolve_delta;
    };

    fsm_Event events[Num_Robots] = {};
    CollisionInfo collision[Num_Robots] = {};

    for (u32 i = 0; i < Num_Robots; i++)
    {
        events[i].is_run_sig = 0;
        events[i].is_wait_sig = 0;
        events[i].is_top_touch = 0;
        events[i].is_bumper = 0;
        events[i].target_switch_pin = 0;
        events[i].elapsed_sim_time = t;

        collision[i].hits = 0;
        collision[i].resolve_delta = vec2(0.0f, 0.0f);

        switch (robots[i].state)
        {
            case Robot_Start:
            {
                if (i < Num_Targets)
                    events[i].target_switch_pin = 1;
                else
                    events[i].target_switch_pin = 0;
            } break;

            case Robot_TargetWait:
            case Robot_ObstacleWait:
            {
                events[i].is_run_sig = 1;
            } break;
        }

        // Check for collisions and compute the average resolve
        // delta vector. The resolve delta will be used to move
        // the robot away so it no longer collides.
        for (u32 n = 0; n < Num_Robots; n++)
        {
            if (i == n)
            {
                continue;
            }
            else
            {
                float x1 = robots[i].model.x;
                float y1 = robots[i].model.y;
                float r1 = robots[i].model.L * 0.5f;
                float x2 = robots[n].model.x;
                float y2 = robots[n].model.y;
                float r2 = robots[n].model.L * 0.5f;
                vec2 difference = vec2(x1 - x2, y1 - y2);
                float L = length(difference);
                float intersection = r2 + r1 - L;
                if (intersection > 0.0f)
                {
                    collision[i].hits++;
                    collision[i].resolve_delta += (difference / L) * intersection * 0.5f;
                }
            }
        }
        if (collision[i].hits > 0)
        {
            collision[i].resolve_delta *= 1.0f / (float)collision[i].hits;
            events[i].is_bumper = 1;
        }
    }

    for (u32 i = 0; i < Num_Robots; i++)
    {
        integrate_robot(&robots[i].model, dt);
        if (collision[i].hits > 0)
        {
            robots[i].model.x += collision[i].resolve_delta.x * 1.05f;
            robots[i].model.y += collision[i].resolve_delta.y * 1.05f;
        }
        robots[i].state = fsm_Update(robots[i].state,
                                     &robots[i].internal,
                                     events[i],
                                     &robots[i].action);
        robots[i].model.vl = robots[i].action.left_wheel;
        robots[i].model.vr = robots[i].action.right_wheel;
    }
}

void draw_robot(GroundRobotModel *r)
{
    vec2 tangent = vec2(cos(r->q), sin(r->q));
    vec2 center = vec2(r->x, r->y);
    vec2 forward = vec2(-tangent.y, tangent.x);
    lines_draw_circle(center, r->L * 0.5f);
    lines_draw_line(center - tangent * r->L * 0.5f,
                    center + tangent * r->L * 0.5f);
    lines_draw_line(center,
                    center + forward * 0.2f);
}

void tick(Input io, float t, float dt)
{
    int speed_multiplier = 20;
    for (u32 i = 0; i < speed_multiplier; i++)
    tick_simulator(t*speed_multiplier, dt);

    float a = (float)WINDOW_HEIGHT/WINDOW_WIDTH;
    lines_set_scale(a/12.0f, 1.0f/12.0f);

    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clearc(0.03f, 0.02f, 0.01f, 1.0f);

    // draw grid
    lines_set_width(2.0f);
    lines_set_color(vec4(1.0f, 0.95f, 0.7f, 0.45f));
    for (u32 i = 0; i <= 20; i++)
    {
        float x = (-1.0f + 2.0f * i / 20.0f) * 10.0f;
        lines_draw_line(x, -10.0f, x, +10.0f);
        lines_draw_line(-10.0f, x, +10.0f, x);
    }

    lines_set_color(vec4(1.0f, 0.35f, 0.11f, 1.0f));
    for (u32 i = 0; i < Num_Targets; i++)
        draw_robot(&targets[i].model);

    lines_set_color(vec4(1.0f, 0.9f, 0.1f, 1.0f));
    for (u32 i = 0; i < Num_Obstacles; i++)
        draw_robot(&obstacles[i].model);
    lines_flush();
}

#include "sumo.cpp"
