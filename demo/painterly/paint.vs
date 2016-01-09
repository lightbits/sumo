#version 150

in vec2 texel;
uniform vec3 center;
uniform vec3 normal;
uniform vec3 albedo;
uniform mat4 projection;
uniform mat4 view;
out vec3 v_position;
out vec3 v_normal;
out vec2 v_texel;
out vec3 v_albedo;

/* Requires the input to be normalised.
 * Doesn’t normalise the output. */
vec3 Orthogonal(vec3 v)
{
    float k = fract(abs(v.x) + 0.5);
    return vec3(-v.y, v.x - k * v.z, k * v.y);
}

void main()
{
    vec3 T = normalize(Orthogonal(normal));
    vec3 B = cross(normal, T);
    v_position = center + 0.15*(texel.x*T + texel.y*B);
    v_normal = normal;
    v_texel = texel;
    v_albedo = albedo;
    gl_Position = projection * view * vec4(v_position, 1.0);
}
