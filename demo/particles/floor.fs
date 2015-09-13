#version 150

in vec2 v_texel;
uniform sampler2D shadow_map;
uniform vec3 sky_bounce;
out vec4 f_color;

void main()
{
    float shadow = texture(shadow_map, v_texel).r;
    vec3 albedo = vec3(1.0, 0.5, 0.2);
    f_color.rgb = mix(albedo * sky_bounce, albedo, shadow);
    f_color.a = 1.0;
}
