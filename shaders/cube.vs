#version 150

in vec3 position;
in vec3 normal;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 v_world;
out vec3 v_normal;

void main()
{
    v_world = (model * vec4(position, 1.0)).xyz;
    v_normal = (model * vec4(normal, 0.0)).xyz;
    gl_Position = projection * view * vec4(v_world, 1.0);
}
