#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define bind_array(buffer) glBindBuffer(GL_ARRAY_BUFFER, buffer)
#define _load_pass(vs, fs) load_render_pass("demo/painterly/" vs, "demo/painterly/" fs)

struct Buffers
{
    GLuint quad;
} buf;

GLuint brush;

struct RenderPasses
{
    RenderPass paint;
    RenderPass blit;
    RenderPass blend;
    RenderPass geometry;
} pass;

#define RES_X 128
#define RES_Y 128

struct Framebuffers
{
    so_Framebuffer average0;
    so_Framebuffer average1;
    so_Framebuffer front;

    so_Framebuffer geometry;
} rts;

void se3_decompose(mat4 se3, mat3 *R, vec3 *p)
{
    *R = m_mat3(se3);
    *p = se3.a4.xyz;
}

// Decomposes the (world->view) transformation into
// a rotation matrix and position vector describing
// the camera rotation and position relative to the
// world frame.
void decompose_view(mat4 view, mat3 *R, vec3 *p)
{
    mat3 R_c, R_w;
    vec3 p_c, p_w;
    se3_decompose(view, &R_c, &p_c);
    R_w = m_transpose(R_c);
    p_w = -R_w * p_c;
    R_w.a1 *= -1.0f;
    R_w.a3 *= -1.0f;
    *R = R_w; // Camera frame relative world frame
    *p = p_w; // Camera position relative world origin
}

bool generated;
void init()
{
    pass.paint = _load_pass("paint.vs", "paint.fs");
    pass.blit = _load_pass("blit.vs", "blit.fs");
    pass.blend = _load_pass("blend.vs", "blend.fs");
    pass.geometry = _load_pass("geometry.vs", "geometry.fs");

    brush = so_load_tex2d("demo/painterly/brush.png");

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

    #if 0
    {
        float f = 0.8f;
        float tn = 2.0f;
        float dt = 1.0f / 60.0f;
        printf("%.4f\n", exp(log(1.0f - 0.8f)/(tn/dt)));
    }
    #endif

    generated = false;

    so_make_fbo_rgba(&rts.average0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);
    so_make_fbo_rgba(&rts.average1, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);
    so_make_fbo_rgba(&rts.front, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);

    {
        GLuint colors[3];
        GLenum targets[3];
        for (u32 i = 0; i < 3; i++)
        {
            targets[i] = GL_TEXTURE_2D;
            colors[i] = so_make_tex2d(0, RES_X, RES_Y,
                                      GL_RGBA32F,
                                      GL_RGB, GL_FLOAT,
                                      GL_NEAREST,
                                      GL_NEAREST,
                                      GL_CLAMP_TO_EDGE,
                                      GL_CLAMP_TO_EDGE);
        }
        so_make_fbo(&rts.geometry, RES_X, RES_Y,
                    3, colors, targets, 0);
    }

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

void tick(Input io, float elapsed_time, float frame_time)
{
    mat4 projection = mat_perspective(PI / 2.5f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 64.0f);

    mat4 view = mat_translate(0.0f, 0.0f, -2.0f) * mat_rotate_x(0.4f) * mat_rotate_y(-0.2f);
    // mat4 view = mat_translate(2.0f, -2.0f, -2.0f) * mat_rotate_x(0.1f) * mat_rotate_y(+0.2f);
    vec3 origin;
    mat3 frame;
    decompose_view(view, &frame, &origin);

    if (!generated)
    {
        begin(&pass.geometry);
        {
            glBindFramebuffer(GL_FRAMEBUFFER, rts.geometry.fbo);
            glViewport(0, 0, rts.geometry.width, rts.geometry.height);
            GLenum draw_buffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2
            };
            glDrawBuffers(3, draw_buffers);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
            blend_mode(false);
            glBindBuffer(GL_ARRAY_BUFFER, buf.quad);
            attribfv("texel", 2, 2, 0);
            uniformf("origin", origin);
            uniformf("aspect", WINDOW_WIDTH / (r32)WINDOW_HEIGHT);
            uniformf("framex", frame.a1);
            uniformf("framey", frame.a2);
            uniformf("framez", frame.a3);
            uniformf("time", elapsed_time);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        {
            GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, draw_buffers);
        }
        generated = false;
    }

    begin(&pass.paint);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, rts.front.fbo);
        glViewport(0, 0, rts.front.width, rts.front.height);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rts.geometry.color[0]);
        uniformi("sampler0", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rts.geometry.color[1]);
        uniformi("sampler1", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, rts.geometry.color[2]);
        uniformi("sampler2", 2);

        glClearColor(0.9, 0.9, 0.9, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        blend_mode(true);
        uniformf("projection", projection);
        uniformf("view", view);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, brush);
        uniformi("brush", 3);

        glBindBuffer(GL_ARRAY_BUFFER, buf.quad);
        attribfv("texel", 2, 2, 0);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, RES_X*RES_Y);
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
