#version 150

in vec3 position;
in vec3 normal;
in vec2 texel;
out vec3 v_normal;
out vec2 v_texel;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    v_texel = texel;
    v_normal = (model * vec4(normal, 0.0)).xyz;
}
