#include "sumo.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#include <stdio.h>

#define LINE_BATCH_MAX_IN_BUFFER 256
struct LineBatchVertex { vec2 p; vec4 c; };
struct LineBatch
{
    GLuint vbo;
    GLuint shader;
    GLuint a_position;
    GLuint a_color;
    GLuint u_scale;
    LineBatchVertex vertices[LINE_BATCH_MAX_IN_BUFFER];
    u32 vertex_count;
    r32 scale;
    vec4 color;
};

static char *SHADER_LINE_BATCH_VS =
    "#version 150                                \n"
    "in vec2 position;                           \n"
    "in vec4 color;                              \n"
    "uniform float scale;                        \n"
    "out vec4 vColor;                            \n"
    "void main()                                 \n"
    "{                                           \n"
    "    vColor = color;                         \n"
    "    gl_Position = vec4(scale*position,      \n"
    "                       0.0, 1.0);           \n"
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

void lines_set_scale(float s)
{
    line_batch.scale = s;
}

void lines_set_color(vec4 color)
{
    line_batch.color = color;
}

void lines_init()
{
    glGenBuffers(1, &line_batch.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, line_batch.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LineBatchVertex) * LINE_BATCH_MAX_IN_BUFFER, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    line_batch.scale = 1.0f;
    line_batch.color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    line_batch.vertex_count = 0;

    line_batch.shader = so_load_shader_vf_from_memory(SHADER_LINE_BATCH_VS, SHADER_LINE_BATCH_FS);
    ASSERT(line_batch.shader != 0);
    line_batch.a_position = glGetAttribLocation(line_batch.shader, "position");
    line_batch.a_color    = glGetAttribLocation(line_batch.shader, "color");
    line_batch.u_scale    = glGetUniformLocation(line_batch.shader, "scale");
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

    glUniform1f(line_batch.u_scale, line_batch.scale);
    glDrawArrays(GL_LINES, 0, line_batch.vertex_count);

    glDisableVertexAttribArray(line_batch.a_position);
    glDisableVertexAttribArray(line_batch.a_color);

    glUseProgram(0);
}

void lines_flush()
{
    lines_draw();
    line_batch.vertex_count = 0;
}

void lines_add_point(vec2 p)
{
    if (line_batch.vertex_count == LINE_BATCH_MAX_IN_BUFFER)
        lines_flush();
    LineBatchVertex v = { };
    v.p = p;
    v.c = line_batch.color;
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

#define NUM_TRACES_X 33
#define NUM_TRACES_Y 33
vec2 points[NUM_TRACES_X * NUM_TRACES_Y];
r32 field_range = 10.0f;

void reset()
{
    clearc(0.0f, 0.0f, 0.0f, 1.0f);

    for (u32 y = 0; y < NUM_TRACES_Y; y++)
    for (u32 x = 0; x < NUM_TRACES_X; x++)
    {
        r32 xf = -1.0f + 2.0f * (x -0.4f + 0.8f*frand()) / (float)(NUM_TRACES_X - 1);
        r32 yf = -1.0f + 2.0f * (y -0.4f + 0.8f*frand()) / (float)(NUM_TRACES_Y - 1);
        points[y * NUM_TRACES_X + x] = vec2(xf, yf) * field_range;
    }
}

void init()
{
    lines_init();
    lines_set_color(vec4(0.00f, 0.63f, 0.69f, 0.13f));
    reset();
}

vec2 f(r32 a, r32 b, r32 c, vec2 x)
{
    return vec2(a*x.x - x.x*x.y,
                b*x.x*x.x - c*x.y);
}

void tick(Input io, float t, float dt)
{
    persist r32 a = 3.0f;
    persist r32 b = 0.8f;
    persist r32 c = 0.6f;
    persist r32 sim_speed = 0.05f;
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // for (u32 i = 0; i < NUM_TRACES_X * NUM_TRACES_Y; i++)
    // {
    //     float dx = field_range / NUM_TRACES_X;
    //     float dy = field_range / NUM_TRACES_Y;
    //     lines_set_color(vec4(0.95f, 0.35f, 0.1f, 0.02f));
    //     lines_draw_rect(points[i] - vec2(dx, dy) * 0.5f, vec2(dx, dy));
    //     lines_draw_line(points[i] - vec2(dx, dy) * 0.1f,
    //                     points[i] + vec2(dx, dy) * 0.1f);
    //     lines_draw_line(points[i] + vec2(-dx, dy) * 0.1f,
    //                     points[i] + vec2(dx, -dy) * 0.1f);
    // }

    for (u32 j = 0; j < 10; j++)
    for (u32 i = 0; i < NUM_TRACES_X * NUM_TRACES_Y; i++)
    {
        vec2 fi = f(a, b, c, points[i]);
        vec2 q = points[i] + fi * sim_speed * dt;
        float blend = mapf(0.0f, 50.0f, length(fi), 0.0f, 1.0f);
        vec4 color = mixf(vec4(0.00f, 0.63f, 0.69f, 0.13f),
                          vec4(1.0f, 0.3f, 0.1f, 0.2f),
                          blend);

        lines_set_color(color);
        lines_draw_line(points[i], q);
        points[i] = q;
    }

    field_range += io.mouse.wheel.y * 0.5f;
    if (io.mouse.wheel.y != 0.0f)
        reset();
    lines_set_scale(1.0f / field_range);
    lines_flush();

    ImGui::NewFrame();
    ImGui::Begin("Parameters");

    float x = (-1.0f + 2.0f * io.mouse.pos.x / (float)WINDOW_WIDTH) * field_range;
    float y = (+1.0f - 2.0f * io.mouse.pos.y / (float)WINDOW_HEIGHT) * field_range;

    char location_text[255];
    sprintf(location_text, "x: %.2f, y: %.2f", x, y);

    ImGui::Text(location_text);
    if (ImGui::SliderFloat("a", &a, 0.1f, 10.0f)) reset();
    if (ImGui::SliderFloat("b", &b, 0.1f, 10.0f)) reset();
    if (ImGui::SliderFloat("c", &c, 0.1f, 10.0f)) reset();
    if (ImGui::SliderFloat("speed", &sim_speed, 0.001f, 0.1f)) reset();
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
