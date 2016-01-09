#include "sumo.h"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 600

RenderPass terrain;
so_Framebuffer fbo;
bool drawn;

void init()
{
    terrain = load_render_pass("demo/mystjam/terrain.vs", "demo/mystjam/terrain.fs");
    so_make_fbo_rgba(&fbo, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA);
    glBindTexture(GL_TEXTURE_2D, fbo.color[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    drawn = 0;
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

void tick(Input io, float t, float dt)
{
    if (!drawn)
    {
        begin(&terrain);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
        glViewport(0, 0, fbo.width, fbo.height);
        clearc(0.0f, 0.0f, 0.0f, 0.0f);
        uniformf("aspect", WINDOW_WIDTH / (r32)WINDOW_HEIGHT);
        so_draw_fullscreen_quad();
        drawn = 1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    debug_draw_texture2D(fbo.color[0]);
}

#include "sumo.cpp"
