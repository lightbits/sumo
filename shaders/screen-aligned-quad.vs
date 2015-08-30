#version 150

in vec2 quadcoord;
// use instanced rendering to offset the quadcoords to object positions
// for now, use uniform
out vec2 v_quad;
uniform vec3 position;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    v_quad = quadcoord;
    mat3 R = transpose(mat3(view));
    vec3 world = position + R * vec3(quadcoord, 0.0);
    gl_Position = projection * view * model * vec4(world, 1.0);
}
