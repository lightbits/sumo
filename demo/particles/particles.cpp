/*
Todo:
* Actual SIMD update of particles
* More complex floor geometry
* Raycast against floor triangles to determine collision
*/

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
    float lifetime[NUM_PARTICLES];
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

void begin(RenderPass *pass, so_Framebuffer *fbo)
{
    if (fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
        glViewport(0, 0, fbo->width, fbo->height);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    begin(pass);
}

void spawn_particle(u32 i)
{
    vec3 color_choices[3] = {
        vec3(0.70, 0.24, 0.11),
        vec3(0.00, 0.35, 0.33),
        vec3(0.77, 0.71, 0.46)
    };

    particles.lifetime[i] = 2.0f + (6.0f - 2.0f) * frand();
    particles.scale[i] = 0.02f + (0.08f - 0.02f) * frand();
    particles.color[i] = color_choices[xor128() % 3];
    particles.position[i] = vec3(-0.1f + 0.2f * frand(),
                                 0.1f + 0.1f * frand(),
                                 -0.1f + 0.2f * frand());
    particles.position[i].z *= 0.4f;
    particles.position[i].x *= 0.4f;
    particles.velocity[i] = vec3(-1.0f + 2.0f * frand(),
                                 1.0f + frand() * 4.0f,
                                 -1.0f + 2.0f * frand());
}

void init()
{
    for (u32 i = 0; i < NUM_PARTICLES; i++)
        spawn_particle(i);

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

    particle_pass = load_render_pass("./demo/particles/particle.vs",
                                     "./demo/particles/particle.fs");
    shadow_pass = load_render_pass("./demo/particles/shadow.vs",
                                   "./demo/particles/shadow.fs");
    debug_pass = load_render_pass("./shaders/debug-draw-rt.vs",
                                  "./shaders/debug-draw-rt.fs");
    floor_pass = load_render_pass("./demo/particles/floor.vs",
                                  "./demo/particles/floor.fs");

    shadow_map = make_shadow_map();
    quad = make_quad();
}

void update_particles(float dt)
{
    for (u32 i = 0; i < NUM_PARTICLES; i++)
    {
        float *lifetime = particles.lifetime + i;
        if (*lifetime < 0.0f)
            spawn_particle(i);
        vec3 *velocity = particles.velocity + i;
        vec3 *position = particles.position + i;
        float *scale = particles.scale + i;

        if (*lifetime < 1.0f)
            *scale *= exp(-2.0f * dt);

        *lifetime -= dt;
        velocity->y -= 5.0f * dt;
        *position += *velocity * dt;
        if (position->y - *scale <= 0.0f)
        {
            velocity->y *= -0.3f;
            velocity->x *= 0.98f;
            velocity->z *= 0.98f;
            position->y = *scale;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer.position);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * NUM_PARTICLES, particles.position);

    glBindBuffer(GL_ARRAY_BUFFER, buffer.scale);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(r32) * NUM_PARTICLES, particles.scale);
}

void tick(Input io, float t, float dt)
{
    update_particles(dt);
    vec3 sky_bounce = vec3(0.2f, 0.25f, 0.35f);
    r32 z_near = 0.1f;
    r32 z_far = 10.0f;
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, z_near, z_far);
    mat4 view = mat_translate(0.0f, -0.2f, -5.0f) * mat_rotate_x(0.3f) * mat_rotate_y(0.4f);

    // todo: Implement so3 inverse of transformation matrix
    mat4 light_projection = mat_ortho_depth(-4.0f, +4.0f, -4.0f, +4.0f, 0.1f, 10.0f);
    mat4 light_view = mat_translate(0.0f, 0.0f, -4.0f) * mat_rotate_x(0.7f) * mat_rotate_y(-0.4f);
    // direction to sun in view-space
    vec3 sun_dir = (view * mat_rotate_y(0.4f) * mat_rotate_x(-0.7f) * vec4(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
    sun_dir *= -1.0f;

    begin(&shadow_pass, &shadow_map);
    blend_mode(true, GL_ONE, GL_ONE, GL_MIN);
    depth_test(false);
    depth_write(false);
    clearc(1, 1, 1, 1);
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

    glBindTexture(GL_TEXTURE_2D, shadow_map.color[0]);
    glActiveTexture(GL_TEXTURE0);

    begin(&floor_pass, 0);
    depth_test(true, GL_LEQUAL);
    depth_write(true);
    blend_mode(false);
    clear(0.95f, 0.97f, 1.0f, 1.0f, 1.0f);
    uniformi("shadow_map", 0);
    uniformf("light_projection", light_projection);
    uniformf("light_view", light_view);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("sky_bounce", sky_bounce);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    begin(&particle_pass, 0);
    uniformi("shadow_map", 0);
    uniformf("light_projection", light_projection);
    uniformf("light_view", light_view);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("sky_bounce", sky_bounce);
    uniformf("sun", sun_dir);
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

    #if 1
    s32 scale = 128;
    blend_mode(false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    begin(&debug_pass);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(shadow_map.target[0], shadow_map.color[0]);

    glViewport(0, 0, scale, scale);
    uniformf("maskr", vec4(1, 1, 1, 1));
    uniformf("maskg", vec4(0, 0, 0, 0));
    uniformf("maskb", vec4(0, 0, 0, 0));
    uniformf("maska", vec4(0, 0, 0, 0));
    uniformi("channel", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glViewport(scale, 0, scale, scale);
    uniformf("maskr", vec4(0, 0, 0, 0));
    uniformf("maskg", vec4(1, 1, 1, 1));
    uniformf("maskb", vec4(0, 0, 0, 0));
    uniformf("maska", vec4(0, 0, 0, 0));
    uniformi("channel", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    #endif
}

#include "sumo.cpp"
