#version 130

in vec4 color;
out vec4 fragColor;

in float transparency;

void main()
{
    fragColor = vec4(color.xyz, transparency);
}

