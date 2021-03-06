static char *SHADER_GUI_VS =
    "#version 150                                                    \n"
    "in vec2 position;                                               \n"
    "in vec2 texel;                                                  \n"
    "in vec4 color;                                                  \n"
    "out vec2 vTexel;                                                \n"
    "out vec4 vColor;                                                \n"
    "uniform mat4 projection;                                        \n"
    "void main()                                                     \n"
    "{                                                               \n"
    "    vTexel = texel;                                             \n"
    "    vColor = color;                                             \n"
    "    gl_Position = projection * vec4(position, 0.0, 1.0);        \n"
    "}                                                               \n";

static char *SHADER_GUI_FS =
    "#version 150                                  \n"
    "in vec2 vTexel;                               \n"
    "in vec4 vColor;                               \n"
    "out vec4 outColor;                            \n"
    "uniform sampler2D tex;                        \n"
    "void main()                                   \n"
    "{                                             \n"
    "    outColor = vColor * texture(tex, vTexel); \n"
    "}                                             \n";

struct Gui
{
    GLuint font;
    GLuint program;
    GLuint vbo;
    GLint u_projection;
    GLint u_tex;
    GLint a_position;
    GLint a_texel;
    GLint a_color;
};

static Gui gui;

static void gui_renderLists(ImDrawList **const cmd_lists, int cmd_lists_count)
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

    glUseProgram(gui.program);
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
    gui.program = so_load_program_from_memory(SHADER_GUI_VS, SHADER_GUI_FS);
    check(gui.program != 0, "Failed to load default GUI shader");

    gui.u_projection = glGetUniformLocation(gui.program, "projection");
    gui.u_tex        = glGetUniformLocation(gui.program, "tex");
    gui.a_position   = glGetAttribLocation(gui.program, "position");
    gui.a_texel      = glGetAttribLocation(gui.program, "texel");
    gui.a_color      = glGetAttribLocation(gui.program, "color");

    // Preallocate a decently sized vertex buffer to hold GUI vertex data
    glGenBuffers(1, &gui.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gui.vbo);
    glBufferData(GL_ARRAY_BUFFER, 10000 * sizeof(ImDrawVert), 0, GL_DYNAMIC_DRAW);

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(window_width, window_height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.0f;
    io.RenderDrawListsFn = gui_renderLists;

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
