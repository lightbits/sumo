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

    #include "prototype.cpp"

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
#define WINDOW_TITLE "Prototype"
#endif

#ifndef WINDOW_FLAGS
#define WINDOW_FLAGS 0
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

#ifndef WINDOW_DOUBLE_BUFFER
#define WINDOW_DOUBLE_BUFFER 1
#endif

#include "imgui/imgui.cpp"
#include "gui.cpp"
#include "sorted_array.cpp"
#include "render_pass.cpp"

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

    SDL_GLContext context = SDL_GL_CreateContext(window);
    check(context, "Failed to create context");

    SDL_GL_SetSwapInterval(1); // Wait for vertical refresh

    glewExperimental = true;
    GLenum glew_status = glewInit();
    check(glew_status == GLEW_OK, "Failed to load OpenGL functions");
    glGetError(); // Catch the stray error that sometimes occurs and ignore it

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    gui_init(WINDOW_WIDTH, WINDOW_HEIGHT);
    init();

    u64 initial_tick = get_tick();
    u64 last_frame_t = initial_tick;
    int running = 1;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            gui_poll(event);
            switch (event.type)
            {
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        running = 0;
                    // RELOAD SHADERS
                    // if (event.key.keysym.sym == SDLK_PRINTSCREEN)
                    //     screenshot();
                    break;
                case SDL_QUIT:
                    running = 0;
                    break;
            }
        }

        float elapsed_time = time_since(initial_tick);
        float delta_time = time_since(last_frame_t);
        last_frame_t = get_tick();

        tick(elapsed_time, delta_time);
        gui_tick(delta_time);
        SDL_GL_SwapWindow(window);

        GLenum e = glGetError();
        check(e == GL_NO_ERROR, "OpenGL failed");
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

    return 0;
}
