#version 150

in vec2 v_coord;
in vec3 v_view_centre;
in vec3 v_color;
in float v_scale;
uniform float z_near;
uniform float z_far;
uniform mat4 projection;
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
    vec3 view_n = vec3(v_coord, z);

    vec3 light0 = sun;
    vec3 light1 = (view * vec4(normalize(vec3(0.3, -2.0, 0.3)), 0.0)).xyz;
    vec3 radiance = vec3(0.0);
    radiance += diffuse(view_n, light0, 1.8 * vec3(1.0, 0.98, 0.95));
    radiance += diffuse(view_n, light1, 0.2 * vec3(1.0, 0.4, 0.2));

    // Compute impostor frag depth and store as ndc for depth testing
    vec3 p = v_view_centre + view_n * v_scale;
    float ndc_depth = (projection[2].z * p.z + projection[3].z) / (-p.z);
    float far = gl_DepthRange.far;
    float near = gl_DepthRange.near;
    gl_FragDepth = near + (ndc_depth * 0.5 + 0.5) * (far - near);

    // Store lit color and linear eye depth normalized to 0..1
    f_color.rgb = v_color * sqrt(radiance);
    f_color.a = (p.z + z_near) / (z_near - z_far);
}
