#version 150
in vec2 texel;
in vec3 position;
in vec3 normal;
out vec2 v_texel;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 v_world;
out vec3 v_normal;
out vec3 v_view;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    v_world = (model * vec4(position, 1.0)).xyz;
    v_view = (view * vec4(v_world, 1.0)).xyz;
    v_normal = normal;
    v_texel = texel;
}
