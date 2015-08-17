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

GLuint make_buffer(GLenum target, GLsizei size, GLvoid *data, GLenum usage)
{
    GLuint result = 0;
    glGenBuffers(1, &result);
    glBindBuffer(target, result);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);
    return result;
}

#endif
