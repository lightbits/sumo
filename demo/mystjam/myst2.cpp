#include "sumo.h"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 400
#define FBO_WIDTH (WINDOW_WIDTH/4)
#define FBO_HEIGHT (WINDOW_HEIGHT/4)

struct Buffers
{
    so_Framebuffer ping;
    so_Framebuffer pong;
} buf;

RenderPass terrain;

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

void init()
{
    so_Framebuffer fbos[2];
    for (int i = 0; i < 2; i++)
    {
        GLuint color[2];
        GLenum target[2];
        for (int j = 0; j < 2; j++)
        {
            target[j] = GL_TEXTURE_2D;
            color[j] = so_make_tex2d(0,
                          FBO_WIDTH, FBO_HEIGHT,
                          GL_RGBA32F,
                          GL_RGBA,
                          GL_FLOAT,
                          GL_NEAREST,
                          GL_NEAREST,
                          GL_CLAMP_TO_EDGE,
                          GL_CLAMP_TO_EDGE);
        }
        so_make_fbo(&fbos[i], FBO_WIDTH, FBO_HEIGHT, 2,
                    color, target, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i].fbo);
        glViewport(0, 0, fbos[i].width, fbos[i].height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    buf.ping = fbos[0];
    buf.pong = fbos[1];

    GLuint program = glCreateProgram();
    glBindFragDataLocation(program, 0, "out0");
    glBindFragDataLocation(program, 1, "out1");
    terrain = load_render_pass(program, "demo/mystjam/terrain2.vs", "demo/mystjam/terrain2.fs");
}

void tick(Input io, float t, float dt)
{
    static bool drawn = false;
    if (!drawn)
    {
        begin(&terrain);
        {
            glBindFramebuffer(GL_FRAMEBUFFER, buf.pong.fbo);
            glViewport(0, 0, buf.pong.width, buf.pong.height);
            GLenum draw_buffers[2] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1
            };
            glDrawBuffers(2, draw_buffers);
            clearc(0, 0, 0, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, buf.ping.color[0]);
            uniformf("aspect", FBO_WIDTH / (r32)FBO_HEIGHT);
            uniformi("sampler0", 0);
            uniformi("steps", 4);
            so_draw_fullscreen_quad();
        }
        drawn = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    debug_draw_texture2D(buf.pong.color[1]);

    so_Framebuffer temp = buf.ping;
    buf.ping = buf.pong;
    buf.pong = temp;
}

#include "sumo.cpp"
