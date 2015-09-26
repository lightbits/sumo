#version 150

in vec3 v_world;
in vec2 v_quadcoord;
in vec4 v_color;
out vec4 f_color;

void main()
{
    f_color = v_color;
    float r2 = dot(v_quadcoord, v_quadcoord);
    f_color.a *= exp2(-3.0 * r2);
}
