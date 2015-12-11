#version 150

in vec3 position;
in vec3 normal;
in vec2 texel;
in vec4 color;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 clip0;
uniform vec4 clip1;
out vec3 v_position;
out vec3 v_normal;
out vec2 v_texel;
out vec4 v_color;

void main()
{
    v_normal = (model * vec4(normal, 0.0)).xyz;
    v_texel = texel;
    v_color = color;
    v_position = (model * vec4(position, 1.0)).xyz;

    gl_ClipDistance[0] = dot(clip0, vec4(v_position, 1.0));
    gl_ClipDistance[1] = dot(clip1, vec4(v_position, 1.0));

    gl_Position = projection * view * model * vec4(position, 1.0);
}
