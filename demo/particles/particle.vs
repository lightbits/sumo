#version 150

in vec2 quadcoord;
in vec3 position;
in vec3 color;
in float scale;
uniform mat4 projection;
uniform mat4 view;
out vec2 v_coord;
out vec3 v_color;
out vec3 v_view_centre;
out float v_scale;

void main()
{
    v_coord = quadcoord;
    v_color = color;
    v_scale = scale;
    mat3 R = transpose(mat3(view));
    vec3 world = position + R * vec3(scale * quadcoord, 0.0);
    v_view_centre = (view * vec4(position, 1.0)).xyz;
    gl_Position = projection * view * vec4(world, 1.0);
}
