#version 150

in vec2 v_texel;
uniform sampler2D channel;
out vec4 f_color;

void main()
{
    float shadow = texture(channel, v_texel).r;
    f_color = vec4(1.0, 0.5, 0.2, 1.0) * exp2(-4.0*shadow);
}
