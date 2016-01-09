#version 150

in vec2 v_texel;
uniform sampler2D sampler0;
out vec4 f_color;

void main()
{
    f_color = texture(sampler0, v_texel);
    // f_color.rgb = pow(f_color.rgb, vec3(0.4545));
}
