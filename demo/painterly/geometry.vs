#version 330
in vec2 texel;
out vec2 v_texel;
void main()
{
    v_texel = texel;
    gl_Position = vec4(texel, 0.0, 1.0);
}
