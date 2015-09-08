#version 150

in vec2 v_texel;
uniform sampler2D channel;
out vec4 f_color;

void main()
{
    float shadow = texture(channel, v_texel).r;
    float blend = exp2(-3.0*shadow);
    vec3 albedo = vec3(1.0, 0.5, 0.2);
    vec3 skyref = vec3(0.2, 0.25, 0.35);
    f_color.rgb = mix(albedo * skyref, albedo, blend);
    f_color.a = 1.0;
}
