#version 150

in vec2 texel;
out vec2 v_texel;

void main()
{
    v_texel = vec2(0.5) + 0.5 * texel;
    gl_Position = vec4(texel, 0.0, 1.0);
}
