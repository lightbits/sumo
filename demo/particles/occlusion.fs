#version 150

in vec2 v_coord;
in vec3 v_occluder_position;
in vec3 v_color;
in float v_scale;
in float v_extrusion_scale;
uniform vec3 sun;
uniform vec2 resolution;
uniform mat4 projection;
uniform float z_near;
uniform float z_far;
uniform sampler2D channel;
out float f_occlusion;

vec3 extract_position(vec2 texel)
{
    vec4 sample = texture(channel, texel);
    float z_norm = sample.w;
    float z_view = (z_near - z_far) * z_norm - z_near;
    float x_ndc = -1.0 + 2.0 * texel.x;
    float y_ndc = -1.0 + 2.0 * texel.y;
    float x_view = -z_view * x_ndc / projection[0].x;
    float y_view = -z_view * y_ndc / projection[1].y;
    return vec3(x_view, y_view, z_view);
}

bool intersect_sphere(vec3 ro, vec3 rd, vec3 centre, float radius)
{
    float a = dot(rd, rd);
    float b = 2.0*dot(ro - centre, rd);
    float c = dot(ro - centre, ro - centre) - radius*radius;
    float d = b*b - 4.0*a*c;
    if (d >= 0.0)
    {
        float t = (-b + sqrt(d)) / (2.0 * a);
        if (t < 0.0)
        {
            t = (-b - sqrt(d)) / (2.0 * a);
            if (t < 0.0)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

void main()
{
    float mask_scale = 1.0 / v_extrusion_scale;
    float r2 = dot(v_coord, v_coord);
    if (r2 < mask_scale)
        discard;
    float z = sqrt(1.0 - r2);
    vec3 view_n = vec3(v_coord, z);

    vec2 test_texel = gl_FragCoord.xy / resolution;
    vec3 test_position = extract_position(test_texel);

    if (intersect_sphere(test_position, sun, v_occluder_position, v_scale))
    {
        f_occlusion = 0.0;
    }
    else
    {
        f_occlusion = 1.0;
    }
}
