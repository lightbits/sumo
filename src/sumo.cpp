/*
Usage
=====
Define the functions
    void init();
    void tick(float t, float dt);

init will be called once,
tick will be called every frame.

t will be the time elapsed since
the call to init. dt will be the
time since the last call to tick.

To override default values, define
    WINDOW_WIDTH
    WINDOW_HEIGHT
    WINDOW_FLAGS

Then include this cpp file, like so

    #include "sumo.cpp"

at the end of your prototyping app.

*/
#define check(x, msg) if (!(x)) { printf("%s\n", msg); getchar(); exit(EXIT_FAILURE); }

#ifndef WINDOW_WIDTH
#define WINDOW_WIDTH 640
#endif

#ifndef WINDOW_HEIGHT
#define WINDOW_HEIGHT 640
#endif

#ifndef WINDOW_TITLE
#define WINDOW_TITLE "Sumo"
#endif

#ifndef WINDOW_FLAGS
#define WINDOW_FLAGS 0
#endif

#ifndef WINDOW_HIDE_CURSOR
#define WINDOW_HIDE_CURSOR 0
#endif

#ifndef MULTISAMPLES
#define MULTISAMPLES 0
#endif

#ifndef WINDOW_DEPTH_BITS
#define WINDOW_DEPTH_BITS 24
#endif

#ifndef WINDOW_STENCIL_BITS
#define WINDOW_STENCIL_BITS 8
#endif

#ifndef WINDOW_GL_MAJOR
#define WINDOW_GL_MAJOR 3
#endif

#ifndef WINDOW_GL_MINOR
#define WINDOW_GL_MINOR 1
#endif

// 1 : Enable double buffering
// 0 : Disable double buffering
#ifndef WINDOW_DOUBLE_BUFFER
#define WINDOW_DOUBLE_BUFFER 1
#endif

// 1 : Enable vsync
// 0 : Disable vsync
#ifndef WINDOW_VSYNC
#define WINDOW_VSYNC 1
#endif

#ifdef FPS_LOCK
#define FRAME_TIME (1.0 / FPS_LOCK)
#define SLEEP_GRANULARITY (0.01)
#endif

#define SO_FBO_IMPLEMENTATION
#include "so_fbo.h"

#define SO_TEXTURE_IMPLEMENTATION
#include "so_texture.h"

#define SO_SHADER_IMPLEMENTATION
#include "so_shader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define SO_MAP_IMPLEMENTATION
#include "so_map.h"

#define SO_NOISE_IMPLEMENTATION
#include "so_noise.h"

#define SO_MESH_IMPLEMENTATION
#include "so_mesh.h"

#define SM_CUBEMAP_IMPLEMENTATION
#include "sm_cubemap.h"

#define SM_ASSET_IMPLEMENTATION
#include "sm_asset.h"

#define SM_PASS_IMPLEMENTATION
#include "sm_pass.h"

#define SM_CAMERA_IMPLEMENTATION
#include "sm_camera.h"

#define SM_LINES_IMPLEMENTATION
#include "sm_lines.h"

#include "opengl/gl_core_4_3.c"
#include "imgui/imgui.cpp"
#include "gui.cpp"

// As mentioned, here we take advantage of single
// compilation unit mode to get the number of counters
// that have actually been placed by the programmer.
#ifdef SUMO_DEBUG
// Allocate one more to ensure we always allocate
// an array of size greater than zero
const int num_debug_counters = __COUNTER__;
DebugCounter global_debug_counters[num_debug_counters+1];
#endif

void panic(const char *msg)
{
    printf("An error occurred: %s\n", msg);
    exit(1);
}

const char *gl_error_message(GLenum error)
{
    switch (error)
    {
    case 0: return "NO_ERROR";
    case 0x0500: return "INVALID_ENUM";
    case 0x0501: return "INVALID_VALUE";
    case 0x0502: return "INVALID_OPERATION";
    case 0x0503: return "STACK_OVERFLOW";
    case 0x0504: return "STACK_UNDERFLOW";
    case 0x0505: return "OUT_OF_MEMORY";
    case 0x0506: return "INVALID_FRAMEBUFFER_OPERATION";
    default: return "UNKNOWN";
    }
}

void assert_gl()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        panic(gl_error_message(error));
}

u64 get_tick()
{
    return SDL_GetPerformanceCounter();
}

float get_elapsed_time(u64 begin, u64 end)
{
    u64 frequency = SDL_GetPerformanceFrequency();
    return (float)(end - begin) / (float)frequency;
}

float time_since(u64 then)
{
    u64 now = get_tick();
    return get_elapsed_time(then, now);
}

void take_screenshot(SDL_Window *window)
{
    static u08 pixels[WINDOW_WIDTH*WINDOW_HEIGHT*3];
    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                 GL_RGB, GL_UNSIGNED_BYTE, pixels);

    char filename[64] = {};
    char *c = filename;
    u32 len = strlen(WINDOW_TITLE);
    for (u32 i = 0; i < m_min(len, 40); i++)
    {
        if (WINDOW_TITLE[i] == ' ') *c = '_';
        else *c = tolower(WINDOW_TITLE[i]);
        c++;
    }
    sprintf(c, "_%d.png", get_tick());

    // Write result flipped vertically, beginning on the last row
    // and moving backwards.
    u08 *end = pixels + WINDOW_WIDTH*WINDOW_HEIGHT*3 - WINDOW_WIDTH*3;
    stbi_write_png(filename, WINDOW_WIDTH,
                   WINDOW_HEIGHT, 3, end, -WINDOW_WIDTH * 3);
}

int main(int argc, char **argv)
{
    check(SDL_Init(SDL_INIT_EVERYTHING) == 0, "Failed to initialize SDL");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, WINDOW_GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, WINDOW_GL_MINOR);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, WINDOW_DOUBLE_BUFFER);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, WINDOW_DEPTH_BITS);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, WINDOW_STENCIL_BITS);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, MULTISAMPLES > 0 ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, MULTISAMPLES);

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | WINDOW_FLAGS);
    check(window, "Failed to create window");

    if (WINDOW_HIDE_CURSOR)
        SDL_ShowCursor(0);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    check(context, "Failed to create context");

    SDL_GL_SetSwapInterval(WINDOW_VSYNC);

    check(ogl_LoadFunctions() != ogl_LOAD_FAILED, "Failed to load OpenGL functions");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    gui_init(WINDOW_WIDTH, WINDOW_HEIGHT);
    init();

    Input input = {};

    u64 initial_tick = get_tick();
    u64 last_frame_t = initial_tick;
    float elapsed_time = 0.0f;
    float delta_time = 1.0f / 60.0f;
    int running = 1;
    while (running)
    {
        for (u32 i = 0; i < SDL_NUM_SCANCODES; i++)
            input.key.released[i] = false;
        input.mouse.left.released = false;
        input.mouse.right.released = false;
        input.mouse.middle.released = false;
        input.mouse.wheel.x = 0.0f;
        input.mouse.wheel.y = 0.0f;
        input.mouse.rel.x = 0.0f;
        input.mouse.rel.y = 0.0f;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            gui_poll(event);
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    input.key.down[event.key.keysym.scancode] = true;
                    break;
                case SDL_KEYUP:
                    input.key.down[event.key.keysym.scancode] = false;
                    input.key.released[event.key.keysym.scancode] = true;
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        running = 0;
                    if (event.key.keysym.sym == SDLK_r)
                    {
                        init();

                        // Reset time
                        initial_tick = get_tick();
                        last_frame_t = initial_tick;
                        elapsed_time = 0.0f;
                        delta_time = 1.0f / 60.0f;
                    }
                    if (event.key.keysym.sym == SDLK_PRINTSCREEN)
                        take_screenshot(window);
                    break;
                case SDL_MOUSEMOTION:
                    input.mouse.pos.x = event.motion.x;
                    input.mouse.pos.y = event.motion.y;
                    input.mouse.rel.x = event.motion.xrel;
                    input.mouse.rel.y = event.motion.yrel;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (SDL_BUTTON_LMASK & event.button.button)
                        input.mouse.left.down = true;
                    if (SDL_BUTTON_RMASK & event.button.button)
                        input.mouse.right.down = true;
                    if (SDL_BUTTON_MMASK & event.button.button)
                        input.mouse.middle.down = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (SDL_BUTTON_LMASK & event.button.button)
                    {
                        input.mouse.left.down = false;
                        input.mouse.left.released = true;
                    }
                    if (SDL_BUTTON_RMASK & event.button.button)
                    {
                        input.mouse.right.down = false;
                        input.mouse.right.released = true;
                    }
                    if (SDL_BUTTON_MMASK & event.button.button)
                    {
                        input.mouse.middle.down = false;
                        input.mouse.middle.released = true;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    input.mouse.wheel.x = event.wheel.x;
                    input.mouse.wheel.y = event.wheel.y;
                    break;
                case SDL_QUIT:
                    running = 0;
                    break;
            }
        }
        input.mouse.ndc.x = -1.0f + 2.0f * input.mouse.pos.x / (float)WINDOW_WIDTH;
        input.mouse.ndc.y = +1.0f - 2.0f * input.mouse.pos.y / (float)WINDOW_HEIGHT;

        SDL_GetWindowSize(window, &input.window_width, &input.window_height);

        tick(input, elapsed_time, delta_time);
        gui_tick(delta_time);
        SDL_GL_SwapWindow(window);

        assert_gl();

        delta_time = time_since(last_frame_t);

        #ifdef FPS_LOCK
        float sleep_time = FRAME_TIME - delta_time;
        if (sleep_time >= 0.0f && sleep_time >= SLEEP_GRANULARITY)
            SDL_Delay((u32)(sleep_time * 1000.0f));
        #endif
        delta_time = time_since(last_frame_t);
        last_frame_t = get_tick();
        elapsed_time = time_since(initial_tick);

        #ifdef SUMO_DEBUG
        for (u32 i = 0; i < num_debug_counters; i++)
        {
            DebugCounter *c = global_debug_counters + i;
            if (c->hits > 0)
            {
                printf("%s (L%d) %llucy %lluh %llucy/h\n",
                       c->function, c->line, c->cycle_count,
                       c->hits, c->cycle_count / c->hits);
            }
            else
            {
                printf("%s 0h\n", c->function);
            }
            c->cycle_count = 0;
            c->hits = 0;
        }
        #endif
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

    return 0;
}
