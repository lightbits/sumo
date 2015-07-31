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

#include <stdint.h>
typedef uint32_t    uint;
typedef uint64_t    uint64;
typedef uint32_t    uint32;
typedef uint16_t    uint16;
typedef uint8_t     uint8;
typedef int32_t     int32;
typedef int16_t     int16;
typedef int8_t      int8;

uint64
get_tick()
{
    return SDL_GetPerformanceCounter();
}

float
get_elapsed_time(uint64 begin, uint64 end)
{
    uint64 frequency = SDL_GetPerformanceFrequency();
    return (float)(end - begin) / (float)frequency;
}

float
time_since(uint64 then)
{
    uint64 now = get_tick();
    return get_elapsed_time(then, now);
}

#define GLSL(src) "#version 150 core\n" #src
static char *SHADER_GUI_VS = GLSL(
    in vec2 position;
    in vec2 texel;
    in vec4 color;
    out vec2 vTexel;
    out vec4 vColor;
    uniform mat4 projection;

    void main()
    {
        vTexel = texel;
        vColor = color;
        gl_Position = projection * vec4(round(position), 0.0, 1.0);
    }
);

static char *SHADER_GUI_FS = GLSL(
    in vec2 vTexel;
    in vec4 vColor;
    out vec4 outColor;
    uniform sampler2D tex;
    void main()
    {
        outColor = vColor * texture(tex, vTexel);
    }
);

struct Gui
{
    GLuint font;
    GLuint shader;
    GLuint vbo;
    GLint u_projection;
    GLint u_tex;
    GLint a_position;
    GLint a_texel;
    GLint a_color;
};

static Gui gui;

static void gui_render_lists(ImDrawList **const cmd_lists, int cmd_lists_count)
{
    if (cmd_lists_count == 0)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gui.font);
    glEnable(GL_SCISSOR_TEST);

    glUseProgram(gui.shader);
    ImGuiIO &io = ImGui::GetIO();

    mat4 projection = mat_ortho_depth(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 0.0, 1.0f);
    glUniformMatrix4fv(gui.u_projection, 1, GL_FALSE, (const GLfloat*)projection.data);
    glUniform1i(gui.u_tex, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gui.vbo);
    for (int i = 0; i < cmd_lists_count; i++)
    {
        const ImDrawList *cmd_list = cmd_lists[i];
        glBufferSubData(GL_ARRAY_BUFFER, 0, cmd_list->vtx_buffer.size() * sizeof(ImDrawVert), (void*)cmd_list->vtx_buffer.begin());

        glEnableVertexAttribArray(gui.a_position);
        glVertexAttribPointer(gui.a_position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const GLvoid*)(0));

        glEnableVertexAttribArray(gui.a_texel);
        glVertexAttribPointer(gui.a_texel, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (const GLvoid*)(8));

        glEnableVertexAttribArray(gui.a_color);
        glVertexAttribPointer(gui.a_color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (const GLvoid*)(16));

        int vtx_offset = 0;
        const ImDrawCmd *pcmd_end = cmd_list->commands.end();
        for (const ImDrawCmd *pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
        {
            glScissor((int)pcmd->clip_rect.x, (int)(io.DisplaySize.y - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
            glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
            vtx_offset += pcmd->vtx_count;
        }
        glDisableVertexAttribArray(gui.a_position);
        glDisableVertexAttribArray(gui.a_texel);
        glDisableVertexAttribArray(gui.a_color);
    }
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
}

void gui_init(int window_width, int window_height)
{
    gui.shader = so_load_shader_vf_from_memory(SHADER_GUI_VS, SHADER_GUI_FS);
    check(gui.shader != 0, "Failed to load default GUI shader");

    gui.u_projection = glGetUniformLocation(gui.shader, "projection");
    gui.u_tex = glGetUniformLocation(gui.shader, "tex");
    gui.a_position = glGetAttribLocation(gui.shader, "position");
    gui.a_texel = glGetAttribLocation(gui.shader, "texel");
    gui.a_color = glGetAttribLocation(gui.shader, "color");

    // Preallocate a decently sized vertex buffer to hold GUI vertex data
    glGenBuffers(1, &gui.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gui.vbo);
    glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(ImDrawVert), 0, GL_DYNAMIC_DRAW);

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(window_width, window_height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.0f;
    io.RenderDrawListsFn = gui_render_lists;

    // Map special purpose keys to the corresponding SDL keys
    io.KeyMap[ImGuiKey_Tab]         = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_Home]        = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End]         = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete]      = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]       = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape]      = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A]           = SDL_SCANCODE_A; // for CTRL+A: select all
    io.KeyMap[ImGuiKey_C]           = SDL_SCANCODE_C; // for CTRL+C: copy
    io.KeyMap[ImGuiKey_V]           = SDL_SCANCODE_V; // for CTRL+V: paste
    io.KeyMap[ImGuiKey_X]           = SDL_SCANCODE_X; // for CTRL+X: cut
    io.KeyMap[ImGuiKey_Y]           = SDL_SCANCODE_Y; // for CTRL+Y: redo
    io.KeyMap[ImGuiKey_Z]           = SDL_SCANCODE_Z; // for CTRL+Z: undo

    const void *png_data;
    unsigned int png_size;
    ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
    gui.font = so_load_png_from_memory(png_data, png_size, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void gui_poll(SDL_Event event)
{
    ImGuiIO &io = ImGui::GetIO();
    switch (event.type)
    {
    case SDL_MOUSEWHEEL:
        io.MouseWheel = event.wheel.y;
        break;
    case SDL_TEXTINPUT:
        io.AddInputCharacter(event.text.text[0]);
        break;
    case SDL_KEYUP:
        io.KeysDown[event.key.keysym.scancode] = false;
        io.KeyCtrl = (event.key.keysym.mod & KMOD_CTRL) != 0;
        io.KeyShift = (event.key.keysym.mod & KMOD_SHIFT) != 0;
        break;
    case SDL_KEYDOWN:
        io.KeysDown[event.key.keysym.scancode] = true;
        io.KeyCtrl = (event.key.keysym.mod & KMOD_CTRL) != 0;
        io.KeyShift = (event.key.keysym.mod & KMOD_SHIFT) != 0;
        break;
    case SDL_MOUSEMOTION:
        io.MousePos.x = event.motion.x;
        io.MousePos.y = event.motion.y;
        break;
    case SDL_MOUSEBUTTONDOWN:
        io.MouseDown[event.button.button - 1] = true;
        break;
    case SDL_MOUSEBUTTONUP:
        io.MouseDown[event.button.button - 1] = false;
        break;
    }
}

void gui_tick(float dt)
{
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = dt;
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

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    gui_init(WINDOW_WIDTH, WINDOW_HEIGHT);
    init();

    uint64 initial_tick = get_tick();
    uint64 last_frame_t = initial_tick;
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
