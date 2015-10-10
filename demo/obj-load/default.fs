#version 150

in vec3 v_world;
in vec2 v_texcoord;
in vec3 v_normal;
out vec4 f_color;

vec3 diffuse(vec3 c, vec3 l, vec3 n)
{
    return max(dot(n, l), 0.0) * c;
}

float stripes()
{
    float h = 0.05;
    float fx = mod(v_texcoord.x * 16.0, 1.0);
    float fy = mod(v_texcoord.y * 16.0, 1.0);
    float sx = (1.0 - smoothstep(0.0, h, fx)) + smoothstep(1.0-h, 1.0, fx);
    float sy = (1.0 - smoothstep(0.0, h, fy)) + smoothstep(1.0-h, 1.0, fy);
    return sx + sy - sx*sy;
}

void main()
{
    vec3 n = normalize(v_normal);
    vec3 color = vec3(0.0);

    vec3 c1 = vec3(1.0, 0.9, 0.82);
    vec3 l1 = normalize(vec3(0.8, 1.0, 0.5));

    vec3 c2 = vec3(0.12, 0.15, 0.25);
    vec3 l2 = normalize(vec3(-1.0, -0.3, -0.5));

    color += diffuse(c1, l1, n);
    color += diffuse(c2, l2, n);

    color *= vec3(1.0 - stripes());

    f_color = vec4(color, 1.0);

    f_color.rgb = sqrt(f_color.rgb); // almost gamma correction
}
