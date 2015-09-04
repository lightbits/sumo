#ifndef _prototype_h_
#define _prototype_h_

#define persist static
#define array_count(x) (sizeof(x) / sizeof(x[0]))

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

#ifdef SUMO_DEBUG
struct DebugCounter
{
    char *file;
    char *function;
    u64 cycle_count;
    u64 hits;
    int line;
};

// Predeclare an array of debug counters
// The array is given a size in sumo.cpp,
// after all possible counters have been
// placed in the code, since then we can
// use the __COUNTER__ macro to determine
// how many counters have been set.
DebugCounter global_debug_counters[];

struct TimedBlock
{
    DebugCounter *counter;
    u64 start;

    TimedBlock(int index, int line, char *file, char *function)
    {
        this->counter = global_debug_counters + index;
        this->counter->function = function;
        this->counter->file = file;
        this->counter->line = line;
        this->start = __rdtsc();
    }

    ~TimedBlock()
    {
        this->counter->cycle_count += __rdtsc() - start;
        this->counter->hits++;
    }
};

#define TIMED_BLOCK TimedBlock timed_block_##__LINE__(__COUNTER__, __LINE__, __FILE__, __FUNCTION__)
#else
#define TIMED_BLOCK
#endif

#define SO_FBO_IMPLEMENTATION
#include "so_fbo.h"

#define SO_TEXTURE_IMPLEMENTATION
#include "so_texture.h"

#define SO_SHADER_IMPLEMENTATION
#include "so_shader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "so_math.h"
#include "so_intrin.h"
#include "noise.h"
#include "imgui/imgui.h"
#include "map.h"
#include "pass.h"

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

void clear(vec4 color, float depth)
{
    clear(color.x, color.y, color.z, color.w, depth);
}

void clearc(vec4 color)
{
    clearc(color.x, color.y, color.z, color.w);
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

GLuint make_quad();
Mesh make_cube();

// TODO: Make mouse pos/rel normalized to [0, 1]?
struct Input
{
    struct Key
    {
        bool down[SDL_NUM_SCANCODES];
        bool released[SDL_NUM_SCANCODES];
    } key;
    struct Mouse
    {
        vec2 pos;
        vec2 rel;
        struct Button
        {
            bool down;
            bool released;
        } left, right, middle;
        struct Wheel
        {
            r32 x; // The amount scrolled horizontally
            r32 y; // The amount scrolled vertically
        } wheel;
    } mouse;
};

mat4 camera_holdclick(Input io, float dt, float move_speed = 1.0f);

enum CubemapFormat
{
    CubemapCrossLR,
    CubemapCrossTB
};
GLuint load_cubemap(char *path,
                    CubemapFormat format,
                    GLenum min_filter,
                    GLenum mag_filter,
                    GLenum wrap_r,
                    GLenum wrap_s,
                    GLenum wrap_t,
                    GLenum internal_format,
                    GLenum data_type,
                    GLenum data_format);

#endif
