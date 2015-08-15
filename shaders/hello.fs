#version 150

in vec2 texel;
out vec4 color;

void main()
{
    color = vec4(texel, 0.5, 1.0);
}
