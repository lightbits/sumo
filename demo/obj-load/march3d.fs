#version 150
in vec3 world;
uniform vec3 camera_dir;
uniform sampler3D channel;
out vec4 f_color;

float field(vec3 p)
{
    vec3 texel = p * 0.5 + vec3(0.5);
    texel += vec3(0.5 / 64.0);
    return texture(channel, texel).r;
}

bool march(vec3 ro, vec3 rd, out vec3 p, out int steps)
{
    float t = 0.0;
    for (int i = 0; i < 24; i++)
    {
        p = ro + t * rd;
        float d = field(p);
        steps = i;
        if (p.x < -1.0 - 1.0/64.0 || p.x > 1.0 + 1.0/64.0 ||
            p.y < -1.0 - 1.0/64.0 || p.y > 1.0 + 1.0/64.0 ||
            p.z < -1.0 - 1.0/64.0 || p.z > 1.0 + 1.0/64.0)
            return false;
        if (d < 2.0 / 64.0)
            return true;
        t += d;
    }
    return false;
}

void main()
{
    vec3 rd = normalize(camera_dir);
    vec3 ro = world;

    vec3 p;
    int i;
    if (march(ro, rd, p, i))
    {
        f_color = vec4(vec3(0.5) + 0.5 * p, 0.5);
        // f_color.rgb *= 0.5 + 0.5 * (i / 16.0);
        // f_color = vec4(i / 16.0);
    }
    else
    {
        f_color = vec4(0.0);
    }
}
