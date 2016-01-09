#version 150

in vec2 v_texel;
uniform sampler2D samplerPrev;
uniform sampler2D samplerCurr;
uniform float factor;
out vec4 f_color;

void main()
{
    vec4 prev = texture(samplerPrev, v_texel);
    vec4 curr = texture(samplerCurr, v_texel);
    f_color = factor * prev + (1.0 - factor) * curr;
}
