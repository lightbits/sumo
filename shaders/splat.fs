#version 150

in vec3 v_world;
in vec2 v_texel;
out vec4 f_color;

void main()
{
    f_color = vec4(v_texel, 0.5, 1.0);
}
