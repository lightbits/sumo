#version 150

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texel;
in vec3 v_albedo;
uniform sampler2D brush;
out vec4 f_color;

float DIFFUSE(vec3 N, vec3 L)
{
    return max(dot(N, L), 0.0);
}

void main()
{
    #if 0
    vec3 N = normalize(v_normal);
    vec3 L1 = normalize(vec3(1.0));
    vec3 L2 = normalize(vec3(-1.0, -1.0, 1.0));
    vec3 color = vec3(0.0);
    color += DIFFUSE(N, L1) * v_albedo;
    color += 0.2*DIFFUSE(N, L2) * v_albedo;
    #else
    vec3 color = v_albedo;
    #endif

    f_color.rgb = color;
    float r2 = dot(v_texel, v_texel);
    // f_color.a = exp(-4.0*r2);
    f_color.a = texture(brush, vec2(0.5) + 0.5 * v_texel).a;
    f_color.a += exp(-0.8*r2);
    // f_color.a *= exp(-16.0 * r2 * r2);
    // f_color.a = 1.0;
}
