#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *SHADER_VS = GLSL150(
in vec2 position;
out vec2 v_position;
void main()
{
    v_position = position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *SHADER_FS = GLSL150(
in vec2 v_position;
uniform float aspect;
out vec4 f_color;

float sphere(vec3 p, float r)
{
    return length(p) - r;
}

float map(vec3 p)
{
    return sphere(p, 1.0);
}

bool trace_map(vec3 ro, vec3 rd, out vec3 p)
{
    float t = 0.0;
    p = ro;
    for (int i = 0; i < 128; i++)
    {
        p = ro + rd * t;
        float d = map(p);
        if (d < 0.01)
            return true;
        t += d;
    }
    return false;
}

bool trace_floor(vec3 ro, vec3 rd, out vec3 p)
{
    float t = -ro.y / rd.y;
    p = ro + rd * t;
    if (t >= 0.0)
        return true;
    else
        return false;
}

int trace(vec3 ro, vec3 rd, out vec3 p)
{
    if (trace_floor(ro, rd, p))
        return 0;
    else if (trace_map(ro, rd, p))
        return 1;
    else
        return 2;
}

vec3 normal(vec3 p)
{
    vec2 eps = vec2(0.001, 0.0);
    return normalize(vec3(map(p + eps.xyy) - map(p - eps.xyy),
                          map(p + eps.yxy) - map(p - eps.yxy),
                          map(p + eps.yyx) - map(p - eps.yyx)));
}

const vec3 sun_direction = normalize(vec3(1.0, 1.5, 1.0));
vec3 radiance(vec3 ro, vec3 rd)
{
    vec3 p = ro;
    int id = trace(ro, rd, p);
    if (id == 0)
    {
        vec3 n = vec3(0.0, 1.0, 0.0);
        return vec3(0.5) + 0.5 * n;
    }
    else if (id == 1)
    {
        vec3 n = normal(p);
        return vec3(0.5) + 0.5 * n;
    }
    else
    {
        vec3 specular = max(dot(rd, sun_direction), 0.0) * vec3(1.0);
        return specular;
    }
}

void main()
{
    vec2 uv = v_position * vec2(aspect, 1.0);
    vec3 ro = vec3(0.0, 0.2, 4.0);
    vec3 rd = normalize(vec3(uv, -1.0));
    f_color = vec4(radiance(ro, rd), 1.0);
}
);

RenderPass pass;
GLuint quad;

void init()
{
    pass = make_render_pass(SHADER_VS, SHADER_FS);
    quad = make_quad();
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformf("aspect", (float)WINDOW_WIDTH / WINDOW_HEIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#include "sumo.cpp"
