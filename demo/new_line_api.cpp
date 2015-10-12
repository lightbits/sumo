#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *vs = GLSL150(
in vec2 position;
in vec2 corner1;
in vec2 corner2;
in vec4 color;
uniform vec2 scale;
out vec4 v_color;
out vec2 v_position;
flat out vec2 v_corner1;
flat out vec2 v_corner2;
void main()
{
    gl_Position = vec4(scale * position, 0.0, 1.0);
    v_color = color;
    v_position = position;
    v_corner1 = corner1;
    v_corner2 = corner2;
}
);

char *fs = GLSL150(
in vec4 v_color;
in vec2 v_position;
flat in vec2 v_corner1;
flat in vec2 v_corner2;
uniform float radius;
uniform float feather;
out vec4 f_color;
void main()
{
    vec2 center = (v_corner1 + v_corner2) * 0.5;
    vec2 dp = v_position - center;
    vec2 normal = v_corner2 - v_corner1;
    vec2 tangent = vec2(-normal.y, normal.x);
    f_color = v_color;
    float r = length(dp);
    if (dot(dp, tangent) >= 0.0)
    {
        f_color.a *= 1.0 - smoothstep(radius - feather * radius, radius, r);
    }
    else
    {
        f_color.a *= 1.0 - smoothstep(radius - feather * radius, radius, abs(dot(dp,normalize(normal))));
    }
}
);

RenderPass pass;

struct Vertex
{
    vec2 position;
    vec2 corner1;
    vec2 corner2;
    vec4 color;
};

#define VERTICES_PER_BATCH 128
struct Batch
{
    u32 used;
    u32 size;
    Vertex vertices[VERTICES_PER_BATCH];
    GLuint buffer;

    struct Mode
    {
        vec4 color;
        vec2 global_scale;
        float width;
        float feather; // In fraction of width
        bool rounded;
    } mode;
} batch;

void flush()
{
    glBindBuffer(GL_ARRAY_BUFFER, batch.buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, batch.used * sizeof(Vertex), batch.vertices);
    begin(&pass);
    attribfv("position", 2, 10, 0);
    attribfv("corner1",  2, 10, 2);
    attribfv("corner2",  2, 10, 4);
    attribfv("color",    4, 10, 6);
    uniformf("radius", batch.mode.width * 0.5f);
    uniformf("scale", batch.mode.global_scale);
    uniformf("feather", batch.mode.feather);
    glDrawArrays(GL_TRIANGLES, 0, batch.used);
    batch.used = 0;
}

void add_vertices(Vertex *vertices, u32 count)
{
    if (batch.used + count > batch.size)
        flush();
    if (batch.used + count > batch.size)
    {
        printf("Geometry too large to fit into buffer\n");
        return;
    }
    for (u32 i = 0; i < count; i++)
        batch.vertices[batch.used + i] = vertices[i];
    batch.used += count;
}

void draw_line(vec2 a, vec2 b)
{
    vec2 ab = b - a;
    vec2 h = normalize(vec2(-ab.y, ab.x)) * batch.mode.width * 0.5f;
    vec2 r = normalize(ab) * batch.mode.width * 0.5f;
    Vertex v1, v2, v3, v4, v5, v6, v7, v8;

    // body
    v1.position = a + h; v1.corner1 = a - h; v1.corner2 = a + h; v1.color = batch.mode.color;
    v2.position = b + h; v2.corner1 = b + h; v2.corner2 = b - h; v2.color = batch.mode.color;
    v3.position = b - h; v3.corner1 = b + h; v3.corner2 = b - h; v3.color = batch.mode.color;
    v4.position = a - h; v4.corner1 = a - h; v4.corner2 = a + h; v4.color = batch.mode.color;

    if (batch.mode.rounded)
    {
        // top cap
        v5.position = b + h + r; v5.corner1 = b + h; v5.corner2 = b - h; v5.color = batch.mode.color;
        v6.position = b - h + r; v6.corner1 = b + h; v6.corner2 = b - h; v6.color = batch.mode.color;

        // bottom cap
        v7.position = a + h - r; v7.corner1 = a - h; v7.corner2 = a + h; v7.color = batch.mode.color;
        v8.position = a - h - r; v8.corner1 = a - h; v8.corner2 = a + h; v8.color = batch.mode.color;

        Vertex vertices[] = {
            v1, v2, v3, v3, v4, v1,
            v2, v5, v6, v6, v3, v2,
            v4, v8, v7, v7, v1, v4
        };
        add_vertices(vertices, 18);
    }
    else
    {
        Vertex vertices[] = {
            v1, v2, v3, v3, v4, v1
        };
        add_vertices(vertices, 6);
    }
}

void init()
{
    pass = make_render_pass(vs, fs);

    batch.used = 0;
    batch.size = VERTICES_PER_BATCH;
    batch.buffer = make_buffer(GL_ARRAY_BUFFER, sizeof(Vertex) * batch.size, 0, GL_DYNAMIC_DRAW);
    batch.mode.color = vec4(1,1,1,1);
    batch.mode.width = 0.1f;
    batch.mode.global_scale.x = (float)WINDOW_HEIGHT / WINDOW_WIDTH;
    batch.mode.global_scale.y = 1.0f;
    batch.mode.rounded = true;
    batch.mode.feather = 0.5f;
}

void tick(Input io, float t, float dt)
{
    batch.mode.width = 0.02f;
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clearc(0.35f, 0.55f, 1.0f, 1.0f);

    draw_line(vec2(0.0f,0.0f), vec2(0.2f, 0.7f));
    draw_line(vec2(0.2f,0.7f), vec2(0.5f, 0.2f));
    draw_line(vec2(-0.5f, 0.2f), vec2(+0.8f, 0.4f));
    flush();
}

#include "sumo.cpp"
