#version 150

in vec2 texel;
out vec4 color;
uniform float blue_color;

void main()
{
    color = vec4(texel, blue_color, 1.0);
}
