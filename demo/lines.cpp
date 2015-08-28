#include "sumo.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

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

#define NUM_TRACES_X 30
#define NUM_TRACES_Y 30
vec2 points[NUM_TRACES_X * NUM_TRACES_Y];

void reset(r32 range)
{
    clearc(0.0f, 0.0f, 0.0f, 1.0f);

    for (u32 y = 0; y < NUM_TRACES_Y; y++)
    for (u32 x = 0; x < NUM_TRACES_X; x++)
    {
        r32 xf = -1.0f + 2.0f * x / (float)(NUM_TRACES_X - 1);
        r32 yf = -1.0f + 2.0f * y / (float)(NUM_TRACES_Y - 1);
        points[y * NUM_TRACES_X + x] = vec2(xf, yf) * range;
    }
}

void init()
{
    lines_init();
    lines_set_color(vec4(0.00f, 0.63f, 0.69f, 0.13f));
    reset(20.0f);
}

vec2 f(r32 a, r32 b, r32 c, vec2 x)
{
    return vec2(a*x.x - x.x*x.y,
                b*x.x*x.x - c*x.y);
}

r32 clampf(r32 x, r32 lo, r32 hi)
{
    if (x < lo)
        return lo;
    else if (x > hi)
        return hi;
    else
        return x;
}

r32 mapf(r32 t0, r32 t1, r32 t, r32 y0, r32 y1)
{
    return clampf(y0 + (y1 - y0) * (t - t0) / (t1 - t0), y0, y1);
}

vec4 mixf(vec4 a, vec4 b, r32 t)
{
    return a + (b - a) * t;
}

void tick(Input io, float t, float dt)
{
    persist r32 a = 1.0f;
    persist r32 b = 1.0f;
    persist r32 c = 10.0f;
    persist r32 range = 20.0f;
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (u32 j = 0; j < 10; j++)
    for (u32 i = 0; i < NUM_TRACES_X * NUM_TRACES_Y; i++)
    {
        vec2 q = points[i] + f(a, b, c, points[i]) * 0.1f*dt;
        float l = length(q - points[i]);
        vec4 color = mixf(vec4(0.00f, 0.63f, 0.69f, 0.13f),
                          vec4(1.0f, 0.3f, 0.1f, 0.2f),
                          mapf(0.0f, 0.05f, l, 0.0f, 1.0f));
        // vec4 color = mixf(vec4(0.00f, 0.0f, 0.0f, 0.0f),
        //                   vec4(1.0f, 0.3f, 0.1f, 0.2f),
        //                   mapf(0.0f, 0.05f, l, 0.0f, 1.0f));
        lines_set_color(color);
        lines_draw_line(points[i], q);
        points[i] = q;
    }

    range += io.mouse.wheel.y * 0.5f;
    if (io.mouse.wheel.y != 0.0f)
        reset(range);
    lines_set_scale(1.0f / range);

    // lines_next_color();
    // lines_set_width(1.0f);
    // lines_draw_line(0.0f, 0.0f, 1.0f, 1.0f);
    // lines_next_color();
    // lines_next_color();
    // lines_draw_rect(vec2(0.0f, 0.0f), vec2(0.2f, 0.2f), t, vec2(1.0f, 1.0f));
    lines_flush();

    ImGui::NewFrame();
    ImGui::Begin("Parameters");
    if (ImGui::SliderFloat("a", &a, 0.1f, 10.0f)) reset(range);
    if (ImGui::SliderFloat("b", &b, 0.1f, 10.0f)) reset(range);
    if (ImGui::SliderFloat("c", &c, 0.1f, 10.0f)) reset(range);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
