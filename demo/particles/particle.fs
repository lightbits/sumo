#version 150

in vec2 v_coord;
in vec3 v_position;
in vec3 v_color;
uniform mat4 view;
uniform vec3 sun;
out vec4 f_color;

vec3 diffuse(vec3 n, vec3 l, vec3 c)
{
    return max(dot(n, l), 0.0) * c;
}

void main()
{
    float r2 = dot(v_coord, v_coord);
    if (r2 >= 1.0)
        discard;
    float z = sqrt(1.0 - r2);
    vec3 n = vec3(v_coord, z);
    n = transpose(mat3(view)) * n;

    // vec3 light0 = normalize(vec3(1.0, 3.0, 1.0));
    vec3 light0 = sun;
    vec3 light1 = normalize(vec3(0.3, -2.0, 0.3));
    vec3 radiance = vec3(0.0);
    radiance += diffuse(n, light0, 1.8 * vec3(1.0, 0.98, 0.95));
    radiance += diffuse(n, light1, 0.2 * vec3(1.0, 0.4, 0.2));
    f_color.rgb = v_color * sqrt(radiance);
    f_color.a = 1.0;
}
