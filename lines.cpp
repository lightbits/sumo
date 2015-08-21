#include "prototype.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

#define LINE_BATCH_MAX_IN_BUFFER 256
#define LINE_BATCH_MAX_COLOR_COUNT 8
struct LineBatchVertex { vec2 p; vec4 c; };
struct LineBatch
{
    GLuint vbo;
    GLuint shader;
    GLuint a_position;
    GLuint a_color;
    LineBatchVertex vertices[LINE_BATCH_MAX_IN_BUFFER];
    u32 vertex_count;
    u32 color_index;
    u32 color_count;
    vec4 color_scheme[LINE_BATCH_MAX_COLOR_COUNT];
};

static char *SHADER_LINE_BATCH_VS =
    "#version 150                                \n"
    "in vec2 position;                           \n"
    "in vec4 color;                              \n"
    "out vec4 vColor;                            \n"
    "void main()                                 \n"
    "{                                           \n"
    "    vColor = color;                         \n"
    "    gl_Position = vec4(position, 0.0, 1.0); \n"
    "}                                           \n";

static char *SHADER_LINE_BATCH_FS =
    "#version 150           \n"
    "in vec4 vColor;        \n"
    "out vec4 outColor;     \n"
    "void main()            \n"
    "{                      \n"
    "    outColor = vColor; \n"
    "}                      \n";

LineBatch line_batch;

void lines_set_width(float w)
{
    glLineWidth(w);
}

void lines_next_color()
{
    line_batch.color_index = (line_batch.color_index + 1) % line_batch.color_count;
}

void lines_init()
{
    glGenBuffers(1, &line_batch.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_batch.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LineBatchVertex) * LINE_BATCH_MAX_IN_BUFFER, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    line_batch.color_index = 0;
    line_batch.color_count = 5;
    line_batch.color_scheme[0] = vec4(0.00f, 0.63f, 0.69f, 1.0f);
    line_batch.color_scheme[1] = vec4(0.92f, 0.41f, 0.25f, 1.0f);
    line_batch.color_scheme[2] = vec4(0.42f, 0.29f, 0.24f, 1.0f);
    line_batch.color_scheme[3] = vec4(0.79f, 0.20f, 0.25f, 1.0f);
    line_batch.color_scheme[4] = vec4(0.93f, 0.79f, 0.32f, 1.0f);
    line_batch.vertex_count = 0;

    line_batch.shader = so_load_shader_vf_from_memory(SHADER_LINE_BATCH_VS, SHADER_LINE_BATCH_FS);
    ASSERT(line_batch.shader != 0);
    line_batch.a_position = glGetAttribLocation(line_batch.shader, "position");
    line_batch.a_color    = glGetAttribLocation(line_batch.shader, "color");
}

void lines_draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, line_batch.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    line_batch.vertex_count * sizeof(LineBatchVertex),
                    line_batch.vertices);
    glUseProgram(line_batch.shader);
    glEnableVertexAttribArray(line_batch.a_position);
    glVertexAttribPointer(line_batch.a_position, 2, GL_FLOAT,
                          GL_FALSE, sizeof(LineBatchVertex),
                          (const GLvoid*)(0));

    glEnableVertexAttribArray(line_batch.a_color);
    glVertexAttribPointer(line_batch.a_color, 4, GL_FLOAT,
                          GL_FALSE, sizeof(LineBatchVertex),
                          (const GLvoid*)(8));

    glDrawArrays(GL_LINES, 0, line_batch.vertex_count);

    glDisableVertexAttribArray(line_batch.a_position);
    glDisableVertexAttribArray(line_batch.a_color);

    glUseProgram(0);
}

void lines_flush()
{
    lines_draw();
    line_batch.color_index = 0;
    line_batch.vertex_count = 0;
}

void lines_add_point(vec2 p)
{
    if (line_batch.vertex_count == LINE_BATCH_MAX_IN_BUFFER)
        lines_flush();
    LineBatchVertex v = { };
    v.p = p;
    v.c = line_batch.color_scheme[line_batch.color_index];
    line_batch.vertices[line_batch.vertex_count++] = v;
}

void lines_add_line(vec2 a, vec2 b)
{
    lines_add_point(a);
    lines_add_point(b);
}

void lines_draw_rect(vec2 a, vec2 b, vec2 c, vec2 d);
void lines_draw_rect(vec2 lower_left, vec2 size, float angle, vec2 centre_of_rotation);
void lines_draw_rect(vec2 lower_left, vec2 size);
void lines_draw_poly(vec2 *points, u32 count);
void lines_draw_line(float x1, float y1, float x2, float y2);
void lines_draw_line(vec2 a, vec2 b);

void lines_draw_poly(vec2 *points, u32 count)
{
    for (u32 i = 0; i < count-1; i++)
        lines_add_line(points[i], points[i+1]);
}

void lines_draw_rect(vec2 a, vec2 b, vec2 c, vec2 d)
{
    lines_add_line(a, b);
    lines_add_line(b, c);
    lines_add_line(c, d);
    lines_add_line(d, a);
}

void lines_draw_rect(vec2 lower_left, vec2 size, float angle, vec2 centre_of_rotation)
{
    float ca = cos(angle);
    float sa = sin(angle);
    vec2 x = vec2(ca, sa);
    vec2 y = vec2(-sa, ca);
    vec2 a = (vec2(0.0f, 0.0f) - centre_of_rotation) * size;
    vec2 b = (vec2(1.0f, 0.0f) - centre_of_rotation) * size;
    vec2 c = (vec2(1.0f, 1.0f) - centre_of_rotation) * size;
    vec2 d = (vec2(0.0f, 1.0f) - centre_of_rotation) * size;
    a = x * a.x + y * a.y;
    b = x * b.x + y * b.y;
    c = x * c.x + y * c.y;
    d = x * d.x + y * d.y;
    a = a + centre_of_rotation * size + lower_left;
    b = b + centre_of_rotation * size + lower_left;
    c = c + centre_of_rotation * size + lower_left;
    d = d + centre_of_rotation * size + lower_left;
    lines_draw_rect(a, b, c, d);
}

void lines_draw_rect(vec2 lower_left, vec2 size)
{
    vec2 a = lower_left;
    vec2 b = lower_left + vec2(size.x, 0.0f);
    vec2 c = lower_left + vec2(size.x, size.y);
    vec2 d = lower_left + vec2(0.0f, size.y);
    lines_draw_rect(a, b, c, d);
}

void lines_draw_line(vec2 a, vec2 b)
{
    lines_add_line(a, b);
}

void lines_draw_line(float x1, float y1, float x2, float y2)
{
    lines_add_line(vec2(x1, y1), vec2(x2, y2));
}

void init()
{
    lines_init();
}

void tick(float t, float dt)
{
    clearc(0.01f, 0.21f, 0.29f, 1.0f);
    lines_next_color();
    lines_set_width(16.0f);
    lines_draw_line(0.0f, 0.0f, 1.0f, 1.0f);
    lines_next_color();
    lines_next_color();
    lines_draw_rect(vec2(0.0f, 0.0f), vec2(0.2f, 0.2f), t, vec2(1.0f, 1.0f));
    lines_flush();
}

#include "prototype.cpp"
