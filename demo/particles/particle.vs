#version 150

in vec2 quadcoord;
in vec3 position;
in vec3 color;
in float scale;
uniform mat4 projection;
uniform mat4 view;
out vec2 v_coord;
out vec3 v_color;
out vec3 v_position;

void main()
{
    v_coord = quadcoord;
    v_color = color;
    mat3 R = transpose(mat3(view));
    vec3 world = position + R * vec3(scale * quadcoord, 0.0);
    v_position = position;
    gl_Position = projection * view * vec4(world, 1.0);
}
