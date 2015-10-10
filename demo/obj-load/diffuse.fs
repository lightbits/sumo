#version 150
in vec2 v_texel;
in vec3 v_world;
in vec3 v_normal;
in vec3 v_view;
uniform sampler2D channel0;
uniform sampler3D channel1;
out vec4 f_color;
#define EPSILON 0.1

float sdf(vec3 p)
{
    return texture(channel1, vec3(0.5) + 0.5 * p).r;
}

float visibility(vec3 ro, vec3 rd)
{
    float t = 0.0;
    float d_min = 1000.0;
    for (int i = 0; i < 4; i++)
    {
        vec3 p = ro + rd * t;
        float d = sdf(p);
        d_min = min(d, d_min);
        if (d < EPSILON)
            return 0.0;
        t += d;
    }
    return smoothstep(0.1, 0.2, d_min);
}

void main()
{
    f_color = texture(channel0, vec2(v_texel.x, 1.0 - v_texel.y));

    vec3 ro = v_world + 0.2 * v_normal;
    vec3 rd = normalize(vec3(0.0, 0.45, 0.0) - ro);
    f_color.rgb = mix(0.98 * f_color.rgb, f_color.rgb, visibility(ro, rd));

    float centerdot = dot(vec3(0.0, 0.0, -1.0), normalize(v_view));
    float depth = clamp((-v_view.z - 3.0) / (5.0 - 3.0), 0.0, 1.0);
    float lat_fade = pow(centerdot, 64.0);
    float lng_fade = 1.0 - smoothstep(0.0, 0.5, depth);
    // f_color.a = 1.0 - lat_fade * lng_fade;

    f_color.rgb *= vec3(1.3, 1.2, 1.1);
    f_color.rgb = smoothstep(vec3(0.0), vec3(0.9, 0.98, 0.95), f_color.rgb);
    f_color.rgb = mix(vec3(0.4, 0.3, 0.2) * f_color.rgb, f_color.rgb, pow(centerdot, 16.0));

    // f_color.rgb *= 0.001;
    // f_color.rgb += vec3(sdf(ro));
    // f_color.rgb = mix(0.25*f_color.rgb, f_color.rgb, sdf(ro));
}
