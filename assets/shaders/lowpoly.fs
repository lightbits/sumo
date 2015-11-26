#version 150

in vec3 v_normal;
in vec2 v_texel;
uniform sampler2D channel0;
out vec4 f_color;

vec3 diffuse(vec3 c, vec3 l, vec3 n)
{
    return max(dot(n, l), 0.0) * c;
}

void main()
{
    vec3 n = normalize(v_normal);
    vec3 color = vec3(0.0);

    vec3 c1 = vec3(1.0, 0.98, 0.92);
    vec3 l1 = normalize(vec3(0.1, 1.0, 0.1));

    vec3 c2 = vec3(0.12, 0.15, 0.25);
    vec3 l2 = normalize(vec3(-1.0, -0.3, -0.5));

    color += diffuse(c1, l1, n);
    color += diffuse(c2, l2, n);

    vec2 texel = vec2(v_texel.x, 1.0 - v_texel.y);
    f_color = texture(channel0, texel) * vec4(color, 1.0);
}
