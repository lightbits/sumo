#version 150

in vec2 v_texel;
uniform sampler2D channel;
uniform vec4 maskr;
uniform vec4 maskg;
uniform vec4 maskb;
uniform vec4 maska;
out vec4 f_color;

void main()
{
    vec4 sample = texture(channel, v_texel);
    f_color = sample.r * maskr +
              sample.g * maskg +
              sample.b * maskb +
              sample.a * maska;
}
