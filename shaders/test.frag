#version 450 core

in vec3 fragNormal;
in vec2 fragTextureCoordinate;

layout(binding = 0) uniform sampler2D texDiffuse;

uniform int render_portals;

void main()
{
    gl_FragColor = texture(texDiffuse, fragTextureCoordinate);//vec4(fragTextureCoordinate, 1.0, 1.0);
}