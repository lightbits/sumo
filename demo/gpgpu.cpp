#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *sh_vertex = GLSL150(
in vec2 position;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *sh_fragment = GLSL150(
uniform sampler2D tex;
layout(pixel_center_integer) in vec4 gl_FragCoord;
out vec4 result;
void main()
{
    ivec2 size = textureSize(tex, 0);
    float du = 1.0 / float(size.x);
    float dv = 1.0 / float(size.y);

    vec2 xy = gl_FragCoord.xy;
    float u = (2.0*xy.x+0.5)*du;
    float v = (2.0*xy.y+0.5)*dv;

    vec2 uv00 = vec2(u, v);
    vec2 uv10 = vec2(u+du, v);
    vec2 uv01 = vec2(u, v+dv);
    vec2 uv11 = vec2(u+du, v+dv);

    vec4 t00 = texture(tex, uv00);
    vec4 t10 = texture(tex, uv10);
    vec4 t01 = texture(tex, uv01);
    vec4 t11 = texture(tex, uv11);

    result = t00 + t10 + t01 + t11;
}
);

void init()
{
    const int size = 4;
    float data[size*size] = {
        0.1f, 0.2f, 0.1f, 0.2f,
        0.3f, 0.4f, 0.3f, 0.4f,
        0.1f, 0.2f, 0.1f, 0.2f,
        0.3f, 0.4f, 0.3f, 0.4f
    };
    float result[size*size*4] = {0};

    GLuint quad = make_quad();
    RenderPass reduce = make_render_pass(sh_vertex, sh_fragment);
    so_Framebuffer fb0, fb1;
    GLenum target = GL_TEXTURE_2D;
    GLuint color0 = so_make_tex2d(data, size, size, GL_RGBA32F, GL_RED, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    GLuint color1 = so_make_tex2d(data, size, size, GL_RGBA32F, GL_RED, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    ASSERT(so_make_fbo(&fb0, size, size, 1, &color0, &target, 0) == true);
    ASSERT(so_make_fbo(&fb1, size, size, 1, &color1, &target, 0) == true);

    so_Framebuffer *src = &fb0;
    so_Framebuffer *dst = &fb1;
    int s = size/2;
    while (s >= 1)
    {
        printf("\ns: %d\n", s);
        glActiveTexture(GL_TEXTURE0);
        glBindFramebuffer(GL_FRAMEBUFFER, dst->fbo);
        glBindTexture(GL_TEXTURE_2D, src->color[0]);
        glViewport(0, 0, s, s);
        begin(&reduce);
        glBindBuffer(GL_ARRAY_BUFFER, quad);
        attribfv("position", 2, 2, 0);
        uniformi("tex", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glReadPixels(0, 0, s, s, GL_RGBA, GL_FLOAT, result);
        for (int y = 0; y < s; y++)
        {
            for (int x = 0; x < s; x++)
                printf("%.4f ", result[(y*s+x)*4+0]);
            printf("\n");
        }

        so_Framebuffer *tmp = src;
        src = dst;
        dst = tmp;

        s = s/2;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void tick(Input io, float t, float dt)
{
}

#include "sumo.cpp"
