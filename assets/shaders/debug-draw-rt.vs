#version 150

in vec2 position;
out vec2 v_texel;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    v_texel = vec2(0.5) + 0.5 * position;
}
