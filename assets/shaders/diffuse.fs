#version 150

in vec3 v_position; // world position
in vec3 v_normal;   // world normal
in vec2 v_texel;
in vec4 v_color;
uniform sampler2D diffuse;
uniform vec4 albedo;
uniform int use_diffuse;
uniform int use_vertex_color;
out vec4 f_color;

void main()
{
    vec2 texel = vec2(v_texel.x, 1.0 - v_texel.y);
    if (use_diffuse == 1 && use_vertex_color == 1)
    {
        f_color = albedo * texture(diffuse, texel) * v_color;
    }
    else if (use_diffuse == 1 && use_vertex_color == 0)
    {
        f_color = albedo * texture(diffuse, texel);
    }
    else if (use_diffuse == 0 && use_vertex_color == 1)
    {
        f_color = albedo * v_color;
    }
    else
    {
        f_color = albedo;
    }
}
