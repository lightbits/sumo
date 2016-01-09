#version 150

in vec2 texel;
uniform sampler2D sampler0;
uniform sampler2D sampler1;
uniform sampler2D sampler2;
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
    ivec2 size = textureSize(sampler0, 0);
    int id = gl_InstanceID;
    vec2 uv;
    uv.x = mod(float(id), float(size.x)) / float(size.x);
    uv.y = float(id) / float(size.y*size.y);

    vec4 data0 = texture(sampler0, uv);
    vec4 data1 = texture(sampler1, uv);
    vec4 data2 = texture(sampler2, uv);

    vec3 center = data0.xyz;
    vec3 albedo = data1.xyz;
    vec3 normal = normalize(data2.xyz);

    // point hit nothing
    if (data0.w < 0.5)
    {
        center = vec3(0.0, 0.0, 10.0);
    }

    vec3 T = normalize(Orthogonal(normal));
    vec3 B = cross(normal, T);
    v_position = center + 0.4*(texel.x*T + texel.y*B);
    v_normal = normal;
    v_texel = texel;
    v_albedo = albedo;
    gl_Position = projection * view * vec4(v_position, 1.0);
}
