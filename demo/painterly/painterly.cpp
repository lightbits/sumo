#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define bind_array(buffer) glBindBuffer(GL_ARRAY_BUFFER, buffer)
#define _load_pass(vs, fs) load_render_pass("demo/painterly/" vs, "demo/painterly/" fs)

struct Buffers
{
    GLuint quad;
} buf;

struct RenderPasses
{
    RenderPass paint;
    RenderPass blit;
    RenderPass blend;
} pass;

#define RES_X 32
#define RES_Y 32
#define NUM_POINTS (RES_X*RES_Y)
struct PointCloud
{
    vec3 center[NUM_POINTS];
    vec3 normal[NUM_POINTS];
    vec3 albedo[NUM_POINTS];
    s32 count;
} cloud;

struct Framebuffers
{
    so_Framebuffer average0;
    so_Framebuffer average1;
    so_Framebuffer front;
} rts;

#include <stdio.h>
void init()
{
    pass.paint = _load_pass("paint.vs", "paint.fs");
    pass.blit = _load_pass("blit.vs", "blit.fs");
    pass.blend = _load_pass("blend.vs", "blend.fs");

    {
        float data[] = {
            -1.0f, -1.0f,
            +1.0f, -1.0f,
            +1.0f, +1.0f,

            +1.0f, +1.0f,
            -1.0f, +1.0f,
            -1.0f, -1.0f
        };
        buf.quad = make_buffer(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    }

    {
        float f = 0.8f;
        float tn = 2.0f;
        float dt = 1.0f / 60.0f;
        printf("%.4f\n", exp(log(1.0f - 0.8f)/(tn/dt)));
    }

    so_make_fbo_rgba(&rts.average0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);
    so_make_fbo_rgba(&rts.average1, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);
    so_make_fbo_rgba(&rts.front, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);

    glBindFramebuffer(GL_FRAMEBUFFER, rts.average0.fbo);
    glViewport(0, 0, rts.average0.width, rts.average0.height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void debug_draw_texture2D(GLuint texture,
                          vec4 maskr = m_vec4(1,0,0,0),
                          vec4 maskg = m_vec4(0,1,0,0),
                          vec4 maskb = m_vec4(0,0,1,0),
                          vec4 maska = m_vec4(0,0,0,1))
{
    static bool loaded = 0;
    static RenderPass pass;
    if (!loaded)
    {
        pass = load_render_pass("assets/shaders/debug-draw-rt.vs",
                                "assets/shaders/debug-draw-rt.fs");
        loaded = 1;
    }
    begin(&pass);
    clearc(0, 0, 0, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    uniformi("channel", 0);
    uniformf("maskr", maskr);
    uniformf("maskg", maskg);
    uniformf("maskb", maskb);
    uniformf("maska", maska);
    so_draw_fullscreen_quad();
}

float SPHERE(vec3 p, float r)
{
    return m_length(p) - r;
}

#define UNITE(expr) d1 = expr; if (d1 < d) d = d1
#define SUBTRACT(expr) d1 = expr; if (-d1 > d) d = -d1;

float MAP(vec3 p)
{
    float d;
    float d1;
    #if 1
    #if 1
    d = SPHERE(p, 0.5f);
    UNITE(SPHERE(p - m_vec3(0.5f, 0.3f, 0.0f), 0.25f));
    UNITE(p.y + 0.2f);
    #else
    d = p.y + 0.2f;
    SUBTRACT(SPHERE(p, 0.5f));
    #endif
    #else
    d = SPHERE(p, 0.5f);
    #endif
    return d;
}

vec3 NORMAL(vec3 p)
{
    vec3 dx = m_vec3(0.01f, 0.0f, 0.0f);
    vec3 dy = m_vec3(0.0f, 0.01f, 0.0f);
    vec3 dz = m_vec3(0.0f, 0.0f, 0.01f);
    return m_normalize(m_vec3(
                       MAP(p + dx) - MAP(p - dx),
                       MAP(p + dy) - MAP(p - dy),
                       MAP(p + dz) - MAP(p - dz)));
}

void tick(Input io, float elapsed_time, float frame_time)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);

    persist vec3 camera_p = m_vec3(0.1f, 0.8f, 4.0f);
    persist quat camera_q = m_quat_from_angle_axis(m_vec3(0,1,0), PI);
    mat4 view = camera_fps(&camera_q, &camera_p, io, frame_time);
    mat3 camera_frame = m_quat_to_so3(camera_q);

    cloud.count = 0;
    {
        vec3 ro = camera_p;
        vec3 forward = camera_frame.a3;
        vec3 up = camera_frame.a2;
        vec3 right = camera_frame.a1;
        r32 pixel_w = 1.0f / (r32)RES_X;
        r32 pixel_h = 1.0f / (r32)RES_Y;
        for (s32 y = 0; y < RES_Y; y++)
        for (s32 x = 0; x < RES_X; x++)
        {
            r32 u = -1.0f + 2.0f * (x + frand()) * pixel_w;
            r32 v = -1.0f + 2.0f * (y + frand()) * pixel_h;
            vec3 rd = m_normalize(forward*1.4f + v*up + u*right);

            bool hit = false;
            vec3 p = ro;
            {
                float t = 0.0f;
                for (s32 i = 0; i < 32; i++)
                {
                    p = ro + rd * t;
                    float d = MAP(p);
                    if (d < 0.01f)
                    {
                        hit = true;
                        break;
                    }
                    t += d;
                }
            }

            if (hit && cloud.count < NUM_POINTS)
            {
                cloud.center[cloud.count] = p;
                cloud.normal[cloud.count] = NORMAL(p);
                cloud.albedo[cloud.count] = m_vec3(1.0f, 0.4f, 0.4f);
                cloud.count++;
            }
        }
    }

    begin(&pass.paint);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, rts.front.fbo);
        glViewport(0, 0, rts.front.width, rts.front.height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        blend_mode(true);
        glBindBuffer(GL_ARRAY_BUFFER, buf.quad);
        attribfv("texel", 2, 2, 0);
        uniformf("projection", projection);
        uniformf("view", view);

        // todo: draw arrays instanced
        // store center, normal, albedo in VBOs
        for (s32 i = 0; i < cloud.count; i++)
        {
            uniformf("center", cloud.center[i]);
            uniformf("normal", cloud.normal[i]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    blend_mode(false);

    begin(&pass.blend);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, rts.average1.fbo);
        glViewport(0, 0, rts.average1.width, rts.average1.height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rts.average0.color[0]);
        uniformi("samplerPrev", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rts.front.color[0]);
        uniformi("samplerCurr", 1);
        uniformf("factor", 0.9867f);
        glBindBuffer(GL_ARRAY_BUFFER, buf.quad);
        attribfv("texel", 2, 2, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    begin(&pass.blit);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rts.average1.color[0]);
        uniformi("sampler0", 0);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, buf.quad);
        attribfv("texel", 2, 2, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    {
        so_Framebuffer temp = rts.average0;
        rts.average0 = rts.average1;
        rts.average1 = temp;
    }
}

#include "sumo.cpp"
