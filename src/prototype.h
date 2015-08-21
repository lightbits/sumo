#ifndef _prototype_h_
#define _prototype_h_

#define persist static

#include <stdint.h>
typedef float       r32;
typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u08;
typedef int32_t     s32;
typedef int16_t     s16;
typedef int8_t      s08;

#define SDL_ASSERT_LEVEL 2

#include "opengl/gl_core_4_3.h"
#include "SDL.h"

#define ASSERT SDL_assert
#include "SDL_assert.h"

#define SO_FBO_IMPLEMENTATION
#include "so_fbo.h"

#define SO_TEXTURE_IMPLEMENTATION
#include "so_texture.h"

#define SO_SHADER_IMPLEMENTATION
#include "so_shader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "so_math.h"

#include "imgui/imgui.h"
#include "sorted_array.h"
#include "render_pass.h"

void clearc(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void cleard(float depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void clear(float r, float g, float b, float a, float depth)
{
    glClearColor(r, g, b, a);
    glClearDepth(depth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void depth_test(bool on, GLenum func)
{
    if (on)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthRangef(0.0f, 1.0f);
        glDepthFunc(func);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void depth_write(bool on)
{
    glDepthMask(on ? GL_TRUE : GL_FALSE);
}

void blend_mode(bool on, GLenum src, GLenum dst)
{
    if (on)
    {
        glEnable(GL_BLEND);
        glBlendFunc(src, dst);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

GLuint make_buffer(GLenum target, GLsizei size, GLvoid *data, GLenum usage)
{
    GLuint result = 0;
    glGenBuffers(1, &result);
    glBindBuffer(target, result);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);
    return result;
}

struct Mesh
{
    GLuint vbo;
    GLuint ibo;
    u32 index_count;
    GLenum index_type;
};

Mesh make_cube()
{
    GLfloat v[] = {
        // Front
        -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f,

        // Back
        +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,

        // Left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

        // Right
        +1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f,

        // Top
        -1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        // Bottom
        +1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f
    };

    GLuint i[] = {
         0,  1,  2,  2,  3,  0,
         4,  5,  6,  6,  7,  4,
         8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    Mesh result = {};
    result.vbo = make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    result.ibo = make_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);
    result.index_count = 36;
    result.index_type = GL_UNSIGNED_INT;
    return result;
}

#endif
