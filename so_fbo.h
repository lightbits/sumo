/* so_fbo - v0.02
Simplified OpenGL framebuffer object handling

How to compile
=======================================================================
This file contains both the header file and the implementation file.
To create the implementation in one source file include this header:

    #define SO_FBO_IMPLEMENTATION
    #include "so_fbo.h"

Make sure that OpenGL symbols are defined before inclusion of the
header.

#define SOI_ASSERT if you do not want to #include "assert.h".

Interface
=======================================================================
    so_make_fbo_rgba:  One 2D color target
    so_make_fbo_rgbad: One 2D color target + depth target
    so_make_fbo:       Up to 8 color targets and up to one depth target

Each function returns true on success and false on failure. Failure
is caused by an incomplete framebuffer. The reasons for this can be
many, and even system dependent. Read up on this at the GL wiki!

color_format must be compatible with glTexImage2D on your system,
i.e. one of the base internal formats in table 1 or sized internal
formats in table 2. (See online documentation).

Usage examples
=======================================================================
Make render target with single 2D texture target
    so_Framebuffer fbo = {};
    so_make_fbo_rgba(fbo, width, height, GL_RGB8);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
    glViewport(0, 0, fbo.width, fbo.height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width, window_height);
    glBindTexture(GL_TEXTURE_2D, fbo.color[0]);

*/

#ifndef SO_FBO_HEADER_INCLUDE
#define SO_FBO_HEADER_INCLUDE
#define SO_FBO_MAX_COLOR_ATTACHMENTS 8

struct so_Framebuffer
{
    GLuint fbo;
    GLuint color[SO_FBO_MAX_COLOR_ATTACHMENTS];
    GLenum target[SO_FBO_MAX_COLOR_ATTACHMENTS];
    GLuint depth;
    int color_count;
    int width;
    int height;
};

extern bool
so_make_fbo(so_Framebuffer *out_result,
            int width, int height,
            int color_count,
            GLuint *color,
            GLenum *target,
            GLuint depth);

extern bool
so_make_fbo_rgbad(so_Framebuffer *out_result,
                  int width, int height,
                  GLenum color_format);

extern bool
so_make_fbo_rgba(so_Framebuffer *out_result,
                 int width, int height,
                 GLenum color_format);

#ifdef SO_FBO_IMPLEMENTATION

// TODO: This is different for GLES
#define SOI_DEPTH_COMPONENT GL_DEPTH_COMPONENT

#ifndef SOI_ASSERT
#include "assert.h"
#define SOI_ASSERT(x) assert(x)
#endif

static GLuint
soi_make_renderbuffer(int width, int height, GLenum format)
{
    GLuint result;
    glGenRenderbuffers(1, &result);
    glBindRenderbuffer(GL_RENDERBUFFER, result);
    glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return result;
}

static GLuint
soi_make_color_target(unsigned char *data,
                      int width, int height,
                      GLenum internal_format,
                      GLenum min_filter,
                      GLenum mag_filter,
                      GLenum wrap_s,
                      GLenum wrap_t)
{
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error = glGetError();
    SOI_ASSERT(error == GL_NO_ERROR);
    return result;
}

bool
so_make_fbo(so_Framebuffer *out_result,
            int width, int height,
            int color_count,
            GLuint *color,
            GLenum *target,
            GLuint depth)
{
    SOI_ASSERT(color_count <= SO_FBO_MAX_COLOR_ATTACHMENTS);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (int i = 0; i < color_count; i++)
    {
        switch (target[i])
        {
            case GL_TEXTURE_2D:
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + i,
                    GL_TEXTURE_2D, color[i], 0);
            } break;

            default:
            {
                // Other targets not yet supported
                return false;
            } break;
        }
    }

    if (depth)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        return false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    so_Framebuffer result = {};
    result.fbo = fbo;
    for (int i = 0; i < color_count; i++)
    {
        result.target[i] = target[i];
        result.color[i] = color[i];
    }
    result.depth = depth;
    result.color_count = color_count;
    result.width = width;
    result.height = height;
    *out_result = result;
    return true;
}

bool
so_make_fbo_rgbad(so_Framebuffer *out_result,
                  int width, int height,
                  GLenum color_format)
{
    GLenum target = GL_TEXTURE_2D;
    GLuint depth = soi_make_renderbuffer(width, height, SOI_DEPTH_COMPONENT);
    GLuint color = soi_make_color_target(0, width, height, color_format,
        GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    return so_make_fbo(out_result, width, height, 1, &color, &target, depth);
}

bool
so_make_fbo_rgba(so_Framebuffer *out_result,
                 int width, int height,
                 GLenum color_format)
{
    GLenum target = GL_TEXTURE_2D;
    GLuint color = soi_make_color_target(0, width, height, color_format,
        GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    return so_make_fbo(out_result, width, height, 1, &color, &target, 0);
}

#endif // SO_FBO_IMPLEMENTATION
#endif // SO_FBO_HEADER_INCLUDE
