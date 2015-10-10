#version 150
in vec2 position;
uniform float depth;
uniform mat4 projection;
uniform mat4 view;
out vec3 v_texel;
void main()
{
    gl_Position = projection * view * vec4(position, depth, 1.0);
    v_texel = vec3(0.5) + 0.5 * vec3(position, depth);
}
