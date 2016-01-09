#version 150

in vec3 v_position;
in vec3 v_normal;
in vec2 v_texel;
out vec4 f_color;

void main()
{
    vec3 N = normalize(v_normal);
    vec3 color = vec3(0.0);
    color += vec3(0.5) + 0.5 * N;
    f_color.rgb = color;
    float r2 = dot(v_texel, v_texel);
    f_color.a = exp(-2.0*r2);
    // f_color.a = 1.0;
}
