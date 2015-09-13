#version 150

in vec2 v_texel;
uniform sampler2D channel_geometry;
uniform sampler2D channel_occlusion;
out vec4 f_color;

void main()
{
    vec3 color = texture(channel_geometry, v_texel).rgb;
    float occlusion = texture(channel_occlusion, v_texel).r;
    f_color.rgb = color * occlusion;
    f_color.a = 1.0;
}
