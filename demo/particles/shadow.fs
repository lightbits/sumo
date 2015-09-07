#version 150

in vec2 v_coord;
out vec4 f_color;

void main()
{
    float r2 = dot(v_coord, v_coord);
    // float alpha = smoothstep(1.0, 0.05, r2);
    float alpha = exp2(-14.0*r2*r2);
    f_color = vec4(alpha * 0.6);
}
