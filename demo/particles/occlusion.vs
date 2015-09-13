#version 150

in vec2 quadcoord;
in vec3 position;
in float scale;
uniform mat4 projection;
uniform mat4 view;
uniform float extrusion_scale;
out vec2 v_coord;
out vec3 v_color;
out vec3 v_occluder_position;
out float v_scale;
out float v_extrusion_scale;

void main()
{
    v_coord = quadcoord;
    v_scale = scale;
    v_extrusion_scale = extrusion_scale;
    mat3 R = transpose(mat3(view));
    vec3 world = position + R * vec3(v_extrusion_scale * scale * quadcoord, 0.0);
    v_occluder_position = (view * vec4(position, 1.0)).xyz;
    gl_Position = projection * view * vec4(world, 1.0);
}
