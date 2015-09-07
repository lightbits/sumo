#version 150

in vec2 quadcoord;
in vec3 position;
in float scale;
uniform mat4 projection;
uniform mat4 view;
out vec2 v_coord;

void main()
{
    v_coord = quadcoord;
    mat3 R = transpose(mat3(view));
    vec3 world = position + R * vec3(2.0*scale * quadcoord, 0.0);
    gl_Position = projection * view * vec4(world, 1.0);
}
