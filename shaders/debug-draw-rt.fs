#version 150

in vec2 v_texel;
uniform sampler2D channel;
uniform vec4 mask;
out vec4 f_color;

void main()
{
    f_color = texture(channel, v_texel) * mask;
}
