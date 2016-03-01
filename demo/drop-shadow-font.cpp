#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
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
uniform mat4 camera;
uniform float aspect;
out vec4 f_color;

float sphere(vec3 p, float r)
{
    return length(p) - r;
}

float map(vec3 p)
{
    return sphere(p - vec3(0.0, 0.2, 0.0), 1.0);
}

bool trace_map(vec3 ro, vec3 rd, out vec3 p)
{
    float t = 0.0;
    p = ro;
    for (int i = 0; i < 32; i++)
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
    vec3 p0;
    vec3 p1;
    bool hit0 = trace_map(ro, rd, p0);
    bool hit1 = trace_floor(ro, rd, p1);
    if (hit0 && hit1)
    {
        if (p0.z < p1.z)
        {
            p = p0;
            return 0;
        }
        else
        {
            p = p1;
            return 1;
        }
    }
    else if (hit0)
    {
        p = p0;
        return 0;
    }
    else if (hit1)
    {
        p = p1;
        return 1;
    }
    else
    {
        return 2;
    }
}

vec3 normal(vec3 p)
{
    vec2 eps = vec2(0.001, 0.0);
    return normalize(vec3(map(p + eps.xyy) - map(p - eps.xyy),
                          map(p + eps.yxy) - map(p - eps.yxy),
                          map(p + eps.yyx) - map(p - eps.yyx)));
}

const vec3 sun_direction = normalize(vec3(1.0, 0.3, 1.0));
vec3 sky(vec3 ro, vec3 rd)
{
    return (0.5 + 0.5*dot(rd, sun_direction)) * vec3(0.85, 0.78, 0.66);
}

vec3 radiance(vec3 ro, vec3 rd)
{
    vec3 p = ro;
    int id = trace(ro, rd, p);
    if (id == 0)
    {
        vec3 n = normal(p);
        float ndotl = max(dot(n, sun_direction), 0.0);
        return vec3(1.0, 0.1, 0.05)*ndotl + vec3(0.02, 0.05, 0.07);
    }
    else if (id == 1)
    {
        float alpha = smoothstep(20.0, 50.0, length(p.xz));
        vec3 ground = mix(vec3(1.0, 0.83, 0.66) * 0.6,
                          sky(ro, rd), alpha);
        ground *= 1.0 - exp2(-2.0 * (dot(p.xz, p.xz) - 0.7));
        return ground;
    }
    else
    {
        return sky(ro, rd);
    }
}

void main()
{
    // vec3 fp = vec3(-v_position.x * aspect,
    //                v_position.y,
    //                1.7);
    vec2 xy = v_position.xy * vec2(-aspect, 1.0);
    float z = sqrt(1.4 - dot(xy, xy));
    vec3 rd = normalize(vec3(xy, z));
    rd = normalize((camera * vec4(rd, 0.0)).xyz);
    vec3 ro = camera[3].xyz;
    f_color = vec4(radiance(ro, rd), 1.0);
    f_color.rgb = sqrt(f_color.rgb);
}
);

RenderPass pass;

void init()
{
    pass = make_render_pass(SHADER_VS, SHADER_FS);
}

void tick(Input io, float t, float dt)
{
    persist quat q = m_quat_from_angle_axis(m_vec3(0, 1, 0), 0.5f);
    persist vec3 p = m_vec3(0, 0.5, -2.0);
    camera_fps(&q, &p, io, dt, 3.0f, 3.0f);

    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    uniformf("aspect", (float)WINDOW_WIDTH / WINDOW_HEIGHT);
    uniformf("camera", m_se3(m_quat_to_so3(q), p));
    so_draw_fullscreen_quad();
}

#include "sumo.cpp"
