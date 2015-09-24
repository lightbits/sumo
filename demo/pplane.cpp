#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *FIELD_VS = GLSL150(
in vec2 position;
out vec2 v_position;

void main()
{
    v_position = position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *FIELD_FS = GLSL150(
in vec2 v_position;
uniform float field_range;
out vec4 f_color;

vec2 eval(vec2 p)
{
    float x = p.x;
    float y = p.y;
    vec2 result;
    result.x = -(x + 2.0*y) * (x + 2.0);
    result.y = -8.0*y*(2.0 + 2.0*x + y);
    return result;
}

void main()
{
    vec2 p = v_position * field_range;
    vec2 f = eval(p);

    float D1 = p.x + 2.0 * p.y + 1.0;
    float D2 = 2.0 * p.x + p.y + 1.0;
    float V = dot(p, p);

    f_color.rg = vec2(0.5) + 0.5 * normalize(f);
    f_color.b = 0.0;
    f_color.a = 1.0;

    // if (V <= 1.0 / 9.0)
    // {
    //     f_color.rgb += vec3(1.0, 0.2, 0.2);
    // }
    // else if (V <= 6.25f)
    // {
    //     f_color.rgb += vec3(0.2, 1.0, 0.2);
    // }

    // if (D1 <= 0.0 || D2 <= 0.0)
    // {
    //     f_color.rgb *= 0.2;
    // }
}
);

char *DYE_VS = GLSL150(
in vec2 quadcoord;
in vec2 position;
in vec3 color;
uniform float field_range;
out vec2 v_quadcoord;
out vec3 v_color;
void main()
{
    v_color = color;
    v_quadcoord = quadcoord;
    gl_Position = vec4(position / field_range + 0.01*quadcoord, 0.0, 1.0);
}
);

char *DYE_FS = GLSL150(
in vec3 v_color;
in vec2 v_quadcoord;
out vec4 f_color;
void main()
{
    float r2 = dot(v_quadcoord, v_quadcoord);
    f_color.rgb = v_color * exp2(-3.0 * r2);
    f_color.a = 1.0;
}
);

RenderPass pass_field, pass_dye;
GLuint quad;

#define Particle_Count 1024
struct Particles
{
    u32 spawn_index;
    vec2 start_position[Particle_Count];
    r32 lifetime[Particle_Count];
    vec2 position[Particle_Count];
    vec3 color[Particle_Count];
} particles;

struct PointBuffer
{
    GLuint position;
    GLuint color;
} buffer;

vec2 eval(vec2 p)
{
    r32 x = p.x;
    r32 y = p.y;
    vec2 result;
    result.x = -(x + 2.0*y) * (x + 2.0);
    result.y = -8.0*y*(2.0 + 2.0*x + y);
    return result;
}

void init()
{
    RenderPassSource source_field = {
        FIELD_VS,
        FIELD_FS
    };
    source_field.from_memory = true;
    pass_field = make_render_pass(source_field);

    RenderPassSource source_dye = {
        DYE_VS,
        DYE_FS
    };
    source_dye.from_memory = true;
    pass_dye = make_render_pass(source_dye);

    quad = make_quad();

    buffer.position = make_buffer(GL_ARRAY_BUFFER,
                                  sizeof(particles.position),
                                  particles.position,
                                  GL_DYNAMIC_DRAW);

    buffer.color = make_buffer(GL_ARRAY_BUFFER,
                              sizeof(particles.color),
                              particles.color,
                              GL_DYNAMIC_DRAW);
}

#define Spawn_Interval 0.1
#define Particle_Lifetime 5.0
#define Spawn_Radius_Noise 0.5
#define Slow_Color vec3(0.00f, 0.63f, 0.69f)
#define Fast_Color vec3(1.00f, 0.30f, 0.10f)
void spawn(r32 x, r32 y, r32 dt, r32 field_range)
{
    persist r32 time_since_last_spawn = Spawn_Interval;

    time_since_last_spawn += dt;
    if (time_since_last_spawn >= Spawn_Interval)
    {
        for (u32 spawn_counter = 0; spawn_counter < 16; spawn_counter++)
        {
            u32 i = particles.spawn_index;
            vec2 *position  = particles.position + i;
            vec3 *color     = particles.color    + i;
            r32 *lifetime   = particles.lifetime + i;
            vec2 *start     = particles.start_position + i;

            r32 theta = frand() * TWO_PI;
            r32 r = frand() * sqrt(Spawn_Radius_Noise * field_range);
            position->x = x + r * cos(theta);
            position->y = y + r * sin(theta);

            *color = Slow_Color;

            *lifetime = Particle_Lifetime;
            *start = *position;
            particles.spawn_index = (i + 1) % Particle_Count;
        }

        time_since_last_spawn = 0.0f;
    }
}

void advect(r32 dt, r32 field_range)
{
    u32 iterations = 10;
    r32 idt = dt / (r32)iterations;
    for (u32 iteration = 0; iteration < iterations; iteration++)
    for (u32 i = 0; i < Particle_Count; i++)
    {
        vec2 f = eval(particles.position[i]) * 0.1f * idt;
        r32 speed = length(f);

        particles.position[i] += f;
        particles.lifetime[i] -= idt;

        particles.color[i] = mixf(Slow_Color,
                                  Fast_Color,
                                  speed / (0.005f * field_range));
        particles.color[i] *= 1.0f - exp(-5.0f * particles.lifetime[i] / Particle_Lifetime);

        if (particles.lifetime[i] <= 0.0f)
        {
            particles.position[i] = particles.start_position[i];
            particles.lifetime[i] = Particle_Lifetime;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(particles.position), particles.position);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.color);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(particles.color), particles.color);
}

void tick(Input io, r32 t, r32 dt)
{
    persist r32 field_range = 2.0f;

    field_range += io.mouse.wheel.y * 0.5f;

    r32 x = (-1.0f + 2.0f * io.mouse.pos.x / (r32)WINDOW_WIDTH) * field_range;
    r32 y = (+1.0f - 2.0f * io.mouse.pos.y / (r32)WINDOW_HEIGHT) * field_range;

    if (io.mouse.left.down)
        spawn(x, y, dt, field_range);

    if (io.key.down['r'])
    {
        for (u32 i = 0; i < Particle_Count; i++)
        {
            particles.position[i] = vec2(0.0f, 0.0f);
            particles.color[i] = vec3(0.0f, 0.0f, 0.0f);
            particles.lifetime[i] = 0.0f;
            particles.start_position[i] = vec2(0.0f, 0.0f);
        }
    }

    advect(dt, field_range);

    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass_field);
    uniformf("field_range", field_range);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    blend_mode(true, GL_ONE, GL_ONE, GL_FUNC_ADD);
    begin(&pass_dye);
    uniformf("field_range", field_range);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("quadcoord", 2, 2, 0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    attribfv("position", 2, 2, 0);
    attribdiv("position", 1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.color);
    attribfv("color", 3, 3, 0);
    attribdiv("color", 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, Particle_Count);
    attribdiv("position", 0);
    attribdiv("color", 0);

    blend_mode(false);
    ImGui::NewFrame();
    ImGui::Begin("Parameters");
    char location_text[255];
    sprintf(location_text, "x: %.2f, y: %.2f", x, y);
    ImGui::Text(location_text);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
