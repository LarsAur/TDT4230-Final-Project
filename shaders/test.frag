#version 450 core

in vec3 fragNormal;
in vec2 fragTextureCoordinate;

layout(binding = 0) uniform sampler2D texDiffuse;

void main()
{
    gl_FragColor = texture(texDiffuse, fragTextureCoordinate);
}