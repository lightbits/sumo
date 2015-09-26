#version 150

#define Epsilon 0.01
#define Steps 64
#define ClipFar 4.0

in vec2 v_position;
uniform float time;
out vec4 f_color;

float sphere(vec3 p, float r)
{
    return length(p) - r;
}

float scene(vec3 p)
{
    float d1 = sphere(p - vec3(-0.5 + 0.25 * cos(time), 0.25 * sin(time), 0.0), 0.5);
    float d2 = sphere(p - vec3(0.5 - 0.25 * cos(time + 0.9), 0.25 * sin(time + 0.9), -1.0), 0.5);
    return min(d1, d2);
}

vec3 normal(vec3 p)
{
    vec2 d = vec2(Epsilon, 0.0);
    float dx = scene(p + d.xyy) - scene(p - d.xyy);
    float dy = scene(p + d.yxy) - scene(p - d.yxy);
    float dz = scene(p + d.yyx) - scene(p - d.yyx);
    return normalize(vec3(dx, dy, dz));
}

bool trace(vec3 ro, vec3 rd, out vec3 p)
{
    vec3 color = vec3(0.0);
    float t = 0.0;
    for (int i = 0; i < Steps; i++)
    {
        p = ro + rd * t;
        float d = scene(p);
        if (d < Epsilon)
            return true;
        t += d;
    }
    return false;
}

void main()
{
    float focal_length = 1.1;
    vec3 u = vec3(0.0, 1.0, 0.0);
    vec3 r = vec3(1.0, 0.0, 0.0);
    vec3 f = vec3(0.0, 0.0, -1.0);
    vec3 ro = vec3(0.0, 0.4, 2.0);
    vec3 rd = normalize(focal_length * f + u * v_position.y + r * v_position.x);

    vec3 p;
    vec3 color = vec3(0.0);
    if (trace(ro, rd, p))
    {
        vec3 n = normal(p);
        color = vec3(0.5) + 0.5 * n;
    }
    f_color.rgb = sqrt(color);
    f_color.a = 1.0;
}
