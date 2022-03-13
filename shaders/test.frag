#version 450 core

in vec3 fragNormal;
in vec2 fragTextureCoordinate;

void main()
{

    gl_FragColor = vec4(fragTextureCoordinate, 1.0, 1.0);
}