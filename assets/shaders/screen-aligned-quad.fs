#version 150

in vec2 v_quad;
out vec4 f_color;

void main()
{
    f_color = vec4(1.0, 0.3, 0.1, 0.5);
    float r2 = dot(v_quad, v_quad);
    if (r2 > 1.0)
        discard;
    float z = sqrt(1.0 - r2);
    vec3 n = vec3(v_quad, z);
    f_color.rgb = vec3(0.5) + 0.5 * n;
    f_color.a = 0.3;
}
