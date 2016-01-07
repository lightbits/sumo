#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define SHADER(src) "#version 150\n" #src

char *sampler_vs = SHADER(
in vec2 position;
out vec2 texel;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    texel = vec2(0.5) + 0.5 * position;
}
);

char *sampler_fs = SHADER(
in vec2 texel;
uniform sampler2D channel;
out vec4 f_color;
void main()
{
    vec4 pal0 = vec4(0.1, 0.06, 0.16, 1.0);
    vec4 pal1 = vec4(0.27, 0.24, 0.47, 1.0);
    vec4 pal2 = vec4(0.83, 0.4, 0.78, 1.0);
    vec4 pal3 = vec4(0.84, 0.73, 0.48, 1.0);
    vec4 pal4 = vec4(0.91, 0.85, 0.63, 1.0);
    vec4 pal5 = vec4(0.62, 0.76, 0.91, 1.0);

    float value = texture(channel, texel).r * 4.0;
    if      (value < 0.2) f_color = pal0;
    else if (value < 1.0) f_color = pal1;
    else if (value < 2.0) f_color = pal2;
    else if (value < 3.0) f_color = pal3;
    else if (value < 4.0) f_color = pal4;
    else                  f_color = pal5;
}
);

char *diffuse_vs = SHADER(
in vec3 position;
in vec3 normal;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 v_position;
out vec3 v_normal;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    v_position = (model * vec4(position, 1.0)).xyz;
    v_normal = (model * vec4(normal, 0.0)).xyz;
}
);

char *diffuse_fs = SHADER(
in vec3 v_normal;
in vec3 v_position;
uniform float albedo;
out vec4 f_color;
void main()
{
    vec3 sun = normalize(vec3(1.0, 5.0, 2.0));
    vec3 n = normalize(v_normal);
    float ndotl = max(dot(sun, n), 0.0);
    f_color.r = ndotl * albedo;
    f_color.r = min(f_color.r, 0.99);
}
);

struct Renders
{
    RenderPass diffuse;
    RenderPass sampler;
    RenderPass blit;
} renders;

struct Framebuffers
{
    so_Framebuffer main;
    so_Framebuffer sampled;
} fbos;

MeshAsset mesh;
GLuint palette;
#define NUM_MESHES 4
vec3 mesh_offsets[NUM_MESHES];

void init()
{
    renders.diffuse = make_render_pass(diffuse_vs, diffuse_fs);
    renders.sampler = make_render_pass(sampler_vs, sampler_fs);
    renders.blit = load_render_pass("assets/shaders/debug-draw-rt.vs",
                                    "assets/shaders/debug-draw-rt.fs");
    so_make_fbo_rgbad(&fbos.main, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB8);
    so_make_fbo_rgba(&fbos.sampled, WINDOW_WIDTH/4, WINDOW_HEIGHT/4, GL_RGB8);
    mesh = load_mesh("assets/models/teapot/teapot.sumo_asset", MESH_NORMALIZE);

    for (u32 i = 0; i < NUM_MESHES; i++)
        mesh_offsets[i] = m_vec3(-1.0f + 2.0f * frand(),
                                 -1.0f + 2.0f * frand(),
                                 -1.0f + 2.0f * frand());
}

void tick(Input io, float t, float dt)
{
    persist quat camera_q = m_quat_from_angle_axis(m_vec3(0, 1, 0), 0.0f);
    persist vec3 camera_position = m_vec3(2.0f, 1.0f, 0);

    mat4 projection = mat_perspective(PI / 3.5f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.02f, 10.0f);
    mat4 view = camera_fps(&camera_q, &camera_position, io, dt, 2.0f);

    begin(&renders.diffuse);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbos.main.fbo);
        glViewport(0, 0, fbos.main.width, fbos.main.height);
        depth_test(true);
        depth_write(true);
        clear(1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

        uniformf("projection", projection);
        uniformf("view", view);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);
        attribfv("position", 3, 3, 0);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.normals);
        attribfv("normal", 3, 3, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);

        for (u32 i = 0; i < NUM_MESHES; i++)
        {
            for (u32 j = 0; j < NUM_MESHES; j++)
            {
                if (i == j)
                    continue;
                vec3 dp = mesh_offsets[i]-mesh_offsets[j];
                r32 k = 1.0f - m_smoothstep(0.4f, 0.55f, m_length(dp));
                mesh_offsets[i] += k * m_normalize(dp) * dt;
            }
            r32 x = mesh_offsets[i].x;
            r32 y = mesh_offsets[i].y;
            r32 z = mesh_offsets[i].z;
            uniformf("model", mat_translate(x, y+0.1f*sin(t+128.0*x), z) *
                              mat_scale(0.5f) *
                              mat_rotate_x(sin(0.3f*t+64.0*x)) *
                              mat_rotate_y(cos(0.2f*t+256.0*y)));
            uniformf("albedo", 1.0f);
            glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);
        }
    }

    begin(&renders.sampler);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbos.sampled.fbo);
        glViewport(0, 0, fbos.sampled.width, fbos.sampled.height);
        depth_test(false);
        depth_write(false);
        clearc(0.0f, 0.0f, 0.0f, 1.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbos.main.color[0]);
        uniformi("channel", 0);
        so_draw_fullscreen_quad();
    }

    begin(&renders.blit);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        clearc(0.0f, 0.0f, 0.0f, 1.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbos.sampled.color[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        uniformi("channel", 0);
        uniformf("maskr", m_vec4(1, 0, 0, 0));
        uniformf("maskg", m_vec4(0, 1, 0, 0));
        uniformf("maskb", m_vec4(0, 0, 1, 0));
        uniformf("maska", m_vec4(0, 0, 0, 1));
        so_draw_fullscreen_quad();
    }
}

#include "sumo.cpp"
