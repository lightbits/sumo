#version 150

in vec2 quadcoord;
uniform int N;
uniform mat4 projection;
uniform mat4 view;
uniform sampler3D tex_sdf;
out vec3 v_world;
out vec2 v_quadcoord;
out vec4 v_color;

vec3 finite_difference_normal(vec3 texel)
{
    vec2 d = vec2(1.0 / N, 0.0);
    float dx = texture(tex_sdf, texel + d.xyy).r - texture(tex_sdf, texel - d.xyy).r;
    float dy = texture(tex_sdf, texel + d.yxy).r - texture(tex_sdf, texel - d.yxy).r;
    float dz = texture(tex_sdf, texel + d.yyx).r - texture(tex_sdf, texel - d.yyx).r;
    vec3 v = vec3(dx, dy, dz);
    float len = length(v);
    if (len != 0.0)
        return v / len;
    else
        return vec3(0.0);
}

// See http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
vec3 ortho(vec3 v)
{
    return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0)
                               : vec3(0.0, -v.z, v.y);
}

void main()
{
    float id = float(gl_InstanceID);
    vec3 grid;
    grid.x = mod(id, N);
    grid.y = mod(floor(id / N), N);
    grid.z = floor(id / (N * N));
    vec3 sdf_texel = (grid + vec3(0.5)) / float(N);

    float sdf = texture(tex_sdf, sdf_texel).r;
    vec3 n = finite_difference_normal(sdf_texel);
    vec3 t = ortho(n);
    vec3 b = cross(n, t);

    vec3 object = quadcoord.x * t + quadcoord.y * b;

    v_world = vec3(-1.0) + 2.0 * (grid + vec3(0.5) + 0.4*object) / float(N);
    v_quadcoord = quadcoord;

    v_color.a = 1.0 - smoothstep(0.0, 0.05, abs(sdf));
    v_color.rgb = vec3(0.5) + 0.5 * n;

    gl_Position = projection * view * vec4(v_world, 1.0);
}
