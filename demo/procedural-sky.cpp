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
uniform sampler2D channel;
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

float clouds(vec3 ro, vec3 rd)
{
    float x = 0.5 + 0.5 * atan(rd.x, rd.z) / 3.1415926;
    float y = rd.y;
    vec2 t = vec2(x, y);
    float n1 = texture(channel, t*0.2).r;
    float n2 = n1 + smoothstep(0.47, 0.49, n1);
    float n3 = n2 * (1.0 - smoothstep(0.5, 0.7, y));
    float n4 = n3 * (1.0 - smoothstep(0.6, 1.0, x));
    float n5 = n4 * (smoothstep(0.0, 0.4, x));
    return n5;
}

const vec3 sun_direction = normalize(vec3(1.0, 0.3, 1.0));
vec3 sky(vec3 ro, vec3 rd)
{
    float n = clouds(ro, rd);

    float s1 = max(dot(rd, sun_direction), 0.0);
    float s2 = pow(s1, 128.0);
    float s3 = pow(s1, 256.0);
    float s4 = pow(1.0-min(abs(rd.y), 1.0), 4.0);
    vec3 c1 = vec3(1.0, 0.83, 0.66);
    vec3 c2 = vec3(0.55, 0.6, 0.63);
    vec3 c3 = mix(c2, c1, s1) + 0.18*vec3(1.5, 1.0, 0.8) * s4 + 0.5*vec3(1.2, 1.0, 0.9) * s3;
    return c3 - 0.4*vec3(0.85, 0.78, 0.66)*n;
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
    vec3 fp = vec3(-v_position.x * aspect,
                   v_position.y,
                   1.7);
    vec3 ro = camera[3].xyz;
    vec3 rd = normalize((camera * vec4(fp, 0.0)).xyz);
    f_color = vec4(radiance(ro, rd), 1.0);
    f_color.rgb = sqrt(f_color.rgb);
}
);

RenderPass pass;
GLuint noise;

void init()
{
    noise = so_load_tex2d("C:/Resources/textures/perlin.png",
                          0, 0, GL_LINEAR, GL_LINEAR,
                          GL_REPEAT, GL_REPEAT);
    pass = make_render_pass(SHADER_VS, SHADER_FS);
}

void tick(Input io, float t, float dt)
{
    persist quat q = m_quat_from_angle_axis(m_vec3(0, 1, 0), 0.5f);
    persist vec3 p = m_vec3(0, 0.5, -2.0);
    camera_fps(&q, &p, io, dt, 3.0f, 3.0f);

    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    glBindTexture(GL_TEXTURE_2D, noise);
    uniformi("channel", 0);
    uniformf("aspect", (float)WINDOW_WIDTH / WINDOW_HEIGHT);
    uniformf("camera", m_se3(m_quat_to_so3(q), p));
    so_draw_fullscreen_quad();
}

#include "sumo.cpp"
