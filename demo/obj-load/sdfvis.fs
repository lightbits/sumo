#version 150
in vec3 v_texel;
uniform float depth;
uniform sampler3D channel;
out vec4 f_color;
void main()
{
    float dist = texture(channel, v_texel).r;
    float seperation = 0.03;
    float modulator = mod(dist / seperation, 1.0);
    float width = 0.006;
    float stripe = smoothstep(1.0 - width/seperation, 1.0, modulator) +
                   1.0 - smoothstep(0.0, width/seperation, modulator);
    f_color.rgb = mix(vec3(1, 0.38, 0.22), 4.0*vec3(1, 0.38, 0.22), stripe);
    f_color.a = 0.5 * stripe;
    f_color.a *= 1.0 - smoothstep(0.0, 0.3, dist);

    // f_color.rgb = vec3(1, 0.38, 0.22) * smoothstep(0.0, 0.1, dist);
    // f_color.a = 0.5;
}
