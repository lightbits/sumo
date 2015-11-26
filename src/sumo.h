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
typedef int64_t     s64;
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

// TODO: Remove when transitioned to new math lib
#ifdef USE_NEW_MATH
#include "so_new_math.h"
#else
#define m_vec2 vec2
#define m_vec4 vec4
#include "so_math.h"
#endif

#include "so_noise.h"
#include "so_map.h"
#include "so_mesh.h"
#include "imgui/imgui.h"

// Map type definitions
// ..............................................................
typedef Map(r32) Map_r32;
typedef Map(u32) Map_u32;
typedef Map(u16) Map_u16;
typedef Map(u08) Map_u08;
typedef Map(s32) Map_s32;
typedef Map(s16) Map_s16;
typedef Map(s08) Map_s08;

// Common file operations
// ..............................................................
char *so_read_file(char *filename)
{
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
    if (!rw)
    {
        // Failed to open file
        ASSERT(false);
    }

    s64 size_in_bytes = SDL_RWsize(rw);

    // Allocate + 1 for null terminator
    char *buffer = (char*)malloc(size_in_bytes + 1);

    s64 bytes_read = 0;
    while (bytes_read < size_in_bytes)
    {
        char *position = buffer + bytes_read;
        s64 remaining = size_in_bytes - bytes_read;
        bytes_read += SDL_RWread(rw, position, 1, remaining);
    }
    SDL_RWclose(rw);
    if (bytes_read != size_in_bytes)
    {
        // Failed to read entire file
        ASSERT(false);
    }
    buffer[bytes_read] = '\0';
    return buffer;
}

// Common gl operations
// ..............................................................
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

struct Input
{
    struct Key
    {
        bool down[SDL_NUM_SCANCODES];
        bool released[SDL_NUM_SCANCODES];
    } key;
    struct Mouse
    {
        vec2 pos; // Position in pixels [0, 0] at top-left window corner
        vec2 ndc; // Position in pixels mapped from [0, w]x[0, h] -> [-1, +1]x[+1, -1]
        vec2 rel; // Movement since last mouse event
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

// Include Sumo APIs
// ..............................................................
#include "sm_cubemap.h"
#include "sm_asset.h"
#include "sm_pass.h"
#include "sm_camera.h"
#include "sm_lines.h"

// Higher level convenience functions that may use the above APIs
// ..............................................................
void so_draw_fullscreen_quad()
{
    persist bool loaded = false;
    persist GLuint buffer = 0;
    if (!loaded)
    {
        float data[] = {
            -1.0f, -1.0f,
            +1.0f, -1.0f,
            +1.0f, +1.0f,
            +1.0f, +1.0f,
            -1.0f, +1.0f,
            -1.0f, -1.0f
        };
        buffer = make_buffer(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        loaded = 1;
    }
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    attribfv("position", 2, 2, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#endif
