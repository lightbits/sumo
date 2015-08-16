#version 400

in vec3 position;
in vec2 texel;
uniform int N;
uniform mat4 projection;
uniform mat4 view;
out vec3 v_world;
out vec2 v_texel;

void main()
{
    #if 0
    float id = float(gl_InstanceID);
    vec3 grid;
    grid.x = mod(id, N);
    grid.y = mod(floor(id / N), N);
    grid.z = floor(id / (N * N));
    v_world = vec3(-1.0) + 2.0 * grid / float(N) + position;
    v_texel = texel;
    #endif
    v_world = position;
    v_texel = texel;
    gl_Position = projection * view * vec4(v_world, 1.0);
}
