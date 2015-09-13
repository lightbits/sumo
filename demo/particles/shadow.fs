#version 150

in vec2 v_coord;
in float v_depth;
out vec2 f_output;

void main()
{
    float r2 = dot(v_coord, v_coord);
    float fade = smoothstep(0.98, 1.0, r2);
    float shadow = 1.0 - exp2(-10.0 * r2 * r2);
    // float shadow = smoothstep(0.0, 1.0, r2);
    f_output = vec2(shadow, mix(v_depth, 1.0, fade));
}
