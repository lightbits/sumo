#version 150

in vec2 quadcoord;
in vec3 position;
in float scale;
uniform mat4 projection;
uniform mat4 view;
out float v_depth;
out vec2 v_coord;

void main()
{
    v_coord = quadcoord;
    mat3 R = transpose(mat3(view));
    vec3 world_position = position + R * vec3(2.0*scale * quadcoord, 0.0);
    vec3 view_position = (view * vec4(world_position, 1.0)).xyz;
    gl_Position = projection * vec4(view_position, 1.0);
    v_depth = 0.5 + 0.5 * gl_Position.z;
}
