#version 150

in vec3 v_world;
in vec2 v_texel;
out vec4 f_color;

void main()
{
    f_color = vec4(vec3(0.5) + 0.5 * v_world, 1.0);
}
