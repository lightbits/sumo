#version 150
in vec3 position;
uniform float scale;
uniform mat4 projection;
uniform mat4 view;
out vec3 world;

void main()
{
    world = scale * position;
    gl_Position = projection * view * vec4(world, 1.0);
}
