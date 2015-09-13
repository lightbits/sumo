#version 150

in vec2 position;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 light_projection;
uniform mat4 light_view;
out vec2 v_texel;

void main()
{
    vec3 world = vec3(4.0 * position.x, 0.0, 4.0 * position.y);
    gl_Position = projection * view * vec4(world, 1.0);

    vec4 light_coord = light_projection * light_view * vec4(world, 1.0);
    light_coord.xy /= light_coord.w;
    v_texel = vec2(0.5) + 0.5 * light_coord.xy;
}
