#ifndef SM_LINES_HEADER_INCLUDE
#define SM_LINES_HEADER_INCLUDE

void lines_set_width(float w);
void lines_set_scale(float s);
void lines_set_scale(float x, float y);
void lines_set_color(u32  color);
void lines_set_color(vec4 color);
void lines_init     ();
void lines_flush    ();
void lines_add_point(vec2 p);
void lines_add_line (vec2 a, vec2 b);
void lines_draw_poly(vec2 *points, u32 count);
void lines_draw_circle(float x, float y, float radius, u32 segments=16);
void lines_draw_circle(vec2 center, float radius, u32 segments=16);
void lines_draw_rect(vec2 a, vec2 b, vec2 c, vec2 d);
void lines_draw_rect(vec2 lower_left, vec2 size, float angle, vec2 centre_of_rotation);
void lines_draw_rect(vec2 lower_left, vec2 size);
void lines_draw_line(vec2 a, vec2 b);
void lines_draw_line(float x1, float y1, float x2, float y2);

#endif // SM_LINES_HEADER_INCLUDE
#ifdef SM_LINES_IMPLEMENTATION
static char *SHADER_LINE_BATCH_VS =
    "#version 150                                \n"
    "in vec2 position;                           \n"
    "in vec4 color;                              \n"
    "uniform vec2 scale;                         \n"
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

#define LINES_VERTICES_PER_BATCH 1024
struct lines_Vertex { vec2 p; vec4 c; };
struct lines_Batch
{
    GLuint vbo;
    GLuint program;
    GLuint a_position;
    GLuint a_color;
    GLuint u_scale;
    lines_Vertex vertices[LINES_VERTICES_PER_BATCH];
    u32 vertex_count;
    vec2 scale;
    vec4 color;
};

lines_Batch lines_batch;

void lines_set_width(float w)
{
    glLineWidth(w);
}

void lines_set_scale(float s)
{
    lines_batch.scale = m_vec2(s, s);
}

void lines_set_scale(float x, float y)
{
    lines_batch.scale = m_vec2(x, y);
}

void lines_set_color(u32 color)
{
    lines_batch.color.x = ((color >> 24) & 0xFF) / 255.0f;
    lines_batch.color.y = ((color >> 16) & 0xFF) / 255.0f;
    lines_batch.color.z = ((color >>  8) & 0xFF) / 255.0f;
    lines_batch.color.w = ((color >>  0) & 0xFF) / 255.0f;
}

void lines_set_color(vec4 color)
{
    lines_batch.color = color;
}

void lines_init()
{
    glGenBuffers(1, &lines_batch.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lines_batch.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines_Vertex) * LINES_VERTICES_PER_BATCH, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    lines_batch.scale = m_vec2(1.0f, 1.0f);
    lines_batch.color = m_vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lines_batch.vertex_count = 0;

    lines_batch.program = so_load_program_from_memory(SHADER_LINE_BATCH_VS, SHADER_LINE_BATCH_FS);
    ASSERT(lines_batch.program != 0);
    lines_batch.a_position = glGetAttribLocation(lines_batch.program, "position");
    lines_batch.a_color    = glGetAttribLocation(lines_batch.program, "color");
    lines_batch.u_scale    = glGetUniformLocation(lines_batch.program, "scale");
}

void lines_draw()
{
    glBindBuffer(GL_ARRAY_BUFFER, lines_batch.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    lines_batch.vertex_count * sizeof(lines_Vertex),
                    lines_batch.vertices);
    glUseProgram(lines_batch.program);
    glEnableVertexAttribArray(lines_batch.a_position);
    glVertexAttribPointer(lines_batch.a_position, 2, GL_FLOAT,
                          GL_FALSE, sizeof(lines_Vertex),
                          (const GLvoid*)(0));

    glEnableVertexAttribArray(lines_batch.a_color);
    glVertexAttribPointer(lines_batch.a_color, 4, GL_FLOAT,
                          GL_FALSE, sizeof(lines_Vertex),
                          (const GLvoid*)(2*sizeof(GLfloat)));

    glUniform2f(lines_batch.u_scale, lines_batch.scale.x, lines_batch.scale.y);
    glDrawArrays(GL_LINES, 0, lines_batch.vertex_count);

    glDisableVertexAttribArray(lines_batch.a_position);
    glDisableVertexAttribArray(lines_batch.a_color);

    glUseProgram(0);
}

void lines_flush()
{
    if (lines_batch.vertex_count > 0)
    {
        lines_draw();
        lines_batch.vertex_count = 0;
    }
}

void lines_add_point(vec2 p)
{
    if (lines_batch.vertex_count + 1 > LINES_VERTICES_PER_BATCH)
        lines_flush();
    lines_Vertex v = { };
    v.p = p;
    v.c = lines_batch.color;
    lines_batch.vertices[lines_batch.vertex_count++] = v;
}

void lines_add_line(vec2 a, vec2 b)
{
    if (lines_batch.vertex_count + 2 > LINES_VERTICES_PER_BATCH)
        lines_flush();
    lines_add_point(a);
    lines_add_point(b);
}

void lines_draw_circle(float x, float y, float radius, u32 segments)
{
    lines_draw_circle(m_vec2(x, y), radius, segments);
}

void lines_draw_circle(vec2 center, float radius, u32 segments)
{
    vec2 a = center + m_vec2(radius, 0.0f);
    for (u32 i = 1; i <= segments; i++)
    {
        float t = TWO_PI * i / (float)segments;
        vec2 b = center + m_vec2(cos(t), sin(t)) * radius;
        lines_add_line(a, b);
        a = b;
    }
}

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
    vec2 x = m_vec2(ca, sa);
    vec2 y = m_vec2(-sa, ca);
    vec2 a = (m_vec2(0.0f, 0.0f) - centre_of_rotation) * size;
    vec2 b = (m_vec2(1.0f, 0.0f) - centre_of_rotation) * size;
    vec2 c = (m_vec2(1.0f, 1.0f) - centre_of_rotation) * size;
    vec2 d = (m_vec2(0.0f, 1.0f) - centre_of_rotation) * size;
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
    vec2 b = lower_left + m_vec2(size.x, 0.0f);
    vec2 c = lower_left + m_vec2(size.x, size.y);
    vec2 d = lower_left + m_vec2(0.0f, size.y);
    lines_draw_rect(a, b, c, d);
}

void lines_draw_line(vec2 a, vec2 b)
{
    lines_add_line(a, b);
}

void lines_draw_line(float x1, float y1, float x2, float y2)
{
    lines_add_line(m_vec2(x1, y1), m_vec2(x2, y2));
}
#endif // SM_LINES_IMPLEMENTATION
