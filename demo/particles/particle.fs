#version 150

in vec2 v_coord;
in vec3 v_world_centre;
in vec3 v_view_centre;
in vec3 v_color;
in float v_scale;
uniform sampler2D shadow_map;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 light_projection;
uniform mat4 light_view;
uniform float light_z_near;
uniform float light_z_far;
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
    vec3 world_n = transpose(mat3(view)) * view_n;
    vec3 view_position = v_view_centre + view_n * v_scale;
    vec3 world_position = v_world_centre + world_n * v_scale;

    vec3 light0 = sun;
    vec3 light1 = (view * vec4(normalize(vec3(0.3, -2.0, 0.3)), 0.0)).xyz;
    vec3 radiance = vec3(0.0);
    radiance += diffuse(view_n, light0, 1.8 * vec3(1.0, 0.98, 0.95));
    radiance += diffuse(view_n, light1, 0.2 * vec3(1.0, 0.4, 0.2));
    f_color.rgb = v_color * sqrt(radiance);

    // Reproject into shadow map and extract occlusion
    vec4 light_view_position = light_view * vec4(world_position, 1.0);
    // TODO: Orthographic projection already does this for you!
    float light_depth = (light_view_position.z + light_z_near) / (light_z_near - light_z_far);

    vec4 light_coord = light_projection * light_view_position;
    light_coord.xy /= light_coord.w;
    vec2 light_texel = vec2(0.5) + 0.5 * light_coord.xy;
    vec2 light_sample = texture(shadow_map, light_texel).rg;
    float shadow = light_sample.r;
    float shadow_depth = light_sample.g;
    if (light_depth > shadow_depth - 0.002)
    {
        vec3 skyref = vec3(0.2, 0.25, 0.35); // TODO: Pass as uniform
        f_color.rgb = mix(f_color.rgb * skyref, f_color.rgb, shadow);
    }

    // Compute impostor frag depth and store as ndc for depth testing
    float ndc_depth = (projection[2].z * view_position.z + projection[3].z) / (-view_position.z);
    float far = gl_DepthRange.far;
    float near = gl_DepthRange.near;
    gl_FragDepth = near + (ndc_depth * 0.5 + 0.5) * (far - near);
}
