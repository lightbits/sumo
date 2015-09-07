#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

so_Framebuffer shadow_map;
RenderPass shadow_pass;
RenderPass particle_pass;
RenderPass floor_pass;
RenderPass debug_pass;
GLuint quad;

struct ParticleBuffer
{
    GLuint position;
    GLuint color;
    GLuint scale;
} buffer;

#define NUM_PARTICLES 256
struct Particles
{
    float scale[NUM_PARTICLES];
    vec3 color[NUM_PARTICLES];
    vec3 position[NUM_PARTICLES];
    vec3 velocity[NUM_PARTICLES];
} particles;

so_Framebuffer make_shadow_map()
{
    GLuint color = so_make_tex2d(0, 512, 512, GL_RGBA32F,
                                 GL_RGBA, GL_FLOAT,
                                 GL_LINEAR, GL_LINEAR,
                                 GL_CLAMP_TO_EDGE,
                                 GL_CLAMP_TO_EDGE);
    GLenum target = GL_TEXTURE_2D;
    so_Framebuffer result = { };
    bool status = so_make_fbo(&result, 512, 512, 1, &color, &target, 0);
    ASSERT(status == true);
    return result;
}

void init()
{
    vec3 color_choices[3] = {
        vec3(0.70, 0.24, 0.11),
        vec3(0.00, 0.35, 0.33),
        vec3(0.77, 0.71, 0.46)
    };

    for (u32 i = 0; i < NUM_PARTICLES; i++)
    {
        int xi = i % 4;
        int yi = (i / 4) % 4;
        int zi = ((i / 4) / 4) % 16;
        particles.scale[i] = 0.04f + (0.08f - 0.04f) * frand();
        particles.color[i] = color_choices[xor128() % 3];
        particles.position[i] = vec3(-1.0f, 0.125f, -1.0f) +
                                vec3(xi / 4.0f, yi / 6.0f, zi / 16.0f) * 2.0f;
        particles.position[i].z *= 0.4f;
        particles.position[i].x *= 0.4f;
        particles.velocity[i] = vec3((-1.0f + 2.0f * frand()) * 1.0f,
                                     (frand()) * 3.0f,
                                     (-1.0f + 2.0f * frand()) * 1.0f);
    }

    buffer.scale = make_buffer(GL_ARRAY_BUFFER,
                               sizeof(float) * NUM_PARTICLES,
                               particles.scale,
                               GL_STATIC_DRAW);

    buffer.position = make_buffer(GL_ARRAY_BUFFER,
                                  sizeof(vec3) * NUM_PARTICLES,
                                  particles.position,
                                  GL_STATIC_DRAW);

    buffer.color = make_buffer(GL_ARRAY_BUFFER,
                               sizeof(vec3) * NUM_PARTICLES,
                               particles.color,
                               GL_STATIC_DRAW);

    RenderPassSource particle_pass_source = {
        "./demo/particles/particle.vs",
        "./demo/particles/particle.fs"
    };
    particle_pass = make_render_pass(particle_pass_source);

    RenderPassSource shadow_pass_source = {
        "./demo/particles/shadow.vs",
        "./demo/particles/shadow.fs"
    };
    shadow_pass = make_render_pass(shadow_pass_source);

    RenderPassSource debug_pass_source = {
        "./shaders/debug-draw-rt.vs",
        "./shaders/debug-draw-rt.fs"
    };
    debug_pass = make_render_pass(debug_pass_source);

    RenderPassSource floor_pass_source = {
        "./demo/particles/floor.vs",
        "./demo/particles/floor.fs"
    };
    floor_pass = make_render_pass(floor_pass_source);

    shadow_map = make_shadow_map();
    quad = make_quad();
}

void update_particles(float dt)
{
    for (u32 i = 0; i < NUM_PARTICLES; i++)
    {
        vec3 *velocity = particles.velocity + i;
        vec3 *position = particles.position + i;
        velocity->y -= 5.0f * dt;
        *position += *velocity * dt;
        if (position->y - particles.scale[i] <= 0.0f)
        {
            velocity->y *= -0.3f;
            velocity->x *= 0.97f;
            velocity->z *= 0.97f;
            position->y = particles.scale[i];
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * NUM_PARTICLES, particles.position);
}

void tick(Input io, float t, float dt)
{
    update_particles(dt);

    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, -0.2f, -5.0f) * mat_rotate_x(0.3f) * mat_rotate_y(0.4f);
    mat4 light_projection = mat_ortho_depth(-4.0f, +4.0f, -4.0f, +4.0f, 0.1f, 10.0f);
    mat4 light_view = mat_translate(0.0f, 0.0f, -4.0f) * mat_rotate_x(0.7f) * mat_rotate_y(-0.4f);
    vec4 sun_dir = mat_rotate_y(0.4f) * mat_rotate_x(-0.7f) * vec4(0.0f, 0.0f, -1.0f, 0.0f);

    blend_mode(true, GL_ONE, GL_ONE);
    depth_test(false, GL_LEQUAL);
    depth_write(false);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.fbo);
    glViewport(0, 0, shadow_map.width, shadow_map.height);
    begin(&shadow_pass);
    clearc(0.0f, 0.0f, 0.0f, 0.0f);
    uniformf("projection", light_projection);
    uniformf("view", light_view);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("quadcoord", 2, 2, 0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    attribfv("position", 3, 3, 0);
    attribdiv("position", 1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.scale);
    attribfv("scale", 1, 1, 0);
    attribdiv("scale", 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, NUM_PARTICLES);

    #if 0
    blend_mode(false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    begin(&debug_pass);
    clearc(0.0f, 0.0f, 0.0f, 0.0f);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    glBindTexture(shadow_map.target[0], shadow_map.color[0]);
    uniformf("mask", vec4(1.0f, 1.0f, 1.0f, 1.0f));
    uniformi("channel", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    #endif

    blend_mode(false);
    depth_test(true, GL_LEQUAL);
    depth_write(true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    clear(0.95f, 0.97f, 1.0f, 1.0f, 1.0f);

    begin(&floor_pass);
    glBindTexture(shadow_map.target[0], shadow_map.color[0]);
    uniformi("channel", 0);
    uniformf("light_projection", light_projection);
    uniformf("light_view", light_view);
    uniformf("projection", projection);
    uniformf("view", view);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    begin(&particle_pass);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("sun", vec3(-sun_dir.x, -sun_dir.y, -sun_dir.z));
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("quadcoord", 2, 2, 0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    attribfv("position", 3, 3, 0);
    attribdiv("position", 1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.color);
    attribfv("color", 3, 3, 0);
    attribdiv("color", 1);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.scale);
    attribfv("scale", 1, 1, 0);
    attribdiv("scale", 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, NUM_PARTICLES);
}

#include "sumo.cpp"
