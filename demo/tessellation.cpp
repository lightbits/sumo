#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 0
#define WINDOW_TITLE "Tessellation"
#define WINDOW_GL_MAJOR 4
#define WINDOW_GL_MINOR 1

#define GLSL400(src) "#version 400\n" #src

char *SHADER_VS = GLSL400(
    in vec3 position;
    out vec3 vsPosition;

    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;

    void main()
    {
        vsPosition = position;
    }
);

char *SHADER_TCS = GLSL400(
    layout(vertices = 4) out;

    in  vec3 vsPosition[];
    out vec3 tcPosition[];

    void main()
    {
        tcPosition[gl_InvocationID] = vsPosition[gl_InvocationID];
        gl_TessLevelInner[0] = 16.0;
        gl_TessLevelInner[1] = 16.0;
        gl_TessLevelOuter[0] = 16.0;
        gl_TessLevelOuter[1] = 16.0;
        gl_TessLevelOuter[2] = 16.0;
        gl_TessLevelOuter[3] = 16.0;
    }
);

char *SHADER_TES = GLSL400(
    layout(quads, equal_spacing, ccw) in;

    in  vec3 tcPosition[];
    out vec3 tePosition;

    void main()
    {
        float u = gl_TessCoord.x;
        float v = gl_TessCoord.y;
        tePosition = mix(
                     mix(tcPosition[0], tcPosition[1], u),
                     mix(tcPosition[3], tcPosition[2], u),
                     v);
        gl_Position = vec4(tePosition, 1.0);
    }
);

char *SHADER_FS = GLSL400(
    out vec4 outColor;

    void main()
    {
        outColor = vec4(1.0);
    }
);

struct Memory
{
    int width;
    int height;

    GLuint program;
    GLint a_position;
    GLint a_texel;
    GLint u_projection;
    GLint u_view;
    GLint u_model;

    GLuint vbo;
};

Memory mem;

void init()
{
    char *sources[] = {
        SHADER_VS,
        SHADER_FS,
        SHADER_TCS,
        SHADER_TES
    };
    GLenum types[] = {
        GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER,
        GL_TESS_CONTROL_SHADER,
        GL_TESS_EVALUATION_SHADER
    };
    mem.program = so_load_shader_from_memory(sources, types, 4);
    Assert(mem.program);

    mem.a_position = glGetAttribLocation(mem.program, "position");
    mem.a_texel = glGetAttribLocation(mem.program, "texel");
    mem.u_projection = glGetUniformLocation(mem.program, "projection");
    mem.u_model = glGetUniformLocation(mem.program, "u_model");
    mem.u_view = glGetUniformLocation(mem.program, "u_view");

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        +0.5f, -0.5f, 0.0f,
        +0.5f, +0.5f, 0.0f,
        -0.5f, +0.5f, 0.0f
    };

    mem.vbo = MakeBuffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void tick(float t, float dt)
{
    glClearColor(0.5f, 0.7f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glUseProgram(mem.program);
    glBindBuffer(GL_ARRAY_BUFFER, mem.vbo);
    glEnableVertexAttribArray(mem.a_position);
    glVertexAttribPointer(mem.a_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    // glUniform2f(mem.u_size, (float)mem.width, (float)mem.height);
    // glUniform1i(mem.u_channel0, 0);
    // glBindTexture(GL_TEXTURE_2D, mem.texture);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawArrays(GL_PATCHES, 0, 4);
    glDisableVertexAttribArray(mem.a_position);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#include "sumo.cpp"
