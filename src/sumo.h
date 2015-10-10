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

#include "opengl/gl_core_4_3.h"

#define SDL_ASSERT_LEVEL 2
#include "SDL.h"

#include "SDL_assert.h"
#define ASSERT SDL_assert

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

#include "so_fbo.h"
#include "so_texture.h"
#include "so_shader.h"
#include "so_math.h"
#include "so_noise.h"
#include "so_map.h"
#include "so_mesh.h"
#include "imgui/imgui.h"

typedef Map(r32) Map_r32;
typedef Map(u32) Map_u32;
typedef Map(u16) Map_u16;
typedef Map(u08) Map_u08;
typedef Map(s32) Map_s32;
typedef Map(s16) Map_s16;
typedef Map(s08) Map_s08;

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

void depth_test(bool on, GLenum func = GL_LEQUAL)
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

void blend_mode(bool on,
                GLenum src = GL_SRC_ALPHA,
                GLenum dst = GL_ONE_MINUS_SRC_ALPHA,
                GLenum mode = GL_FUNC_ADD)
{
    if (on)
    {
        glEnable(GL_BLEND);
        glBlendFunc(src, dst);
        glBlendEquation(mode);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

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

// Include Sumo APIs
#include "sm_pass.h"
#include "sm_camera.h"
#include "sm_lines.h"

#endif
