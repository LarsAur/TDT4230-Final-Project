#version 450 core

in vec3 position;
in vec3 normal;
in vec2 textureCoordinate;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

out vec2 fragTextureCoordinate;
out vec3 fragNormal;
out vec3 fragWorldPos;

void main()
{
    fragNormal = mat3(model) * normal;
    fragWorldPos = (model * vec4(position, 1.0)).xyz;
    fragTextureCoordinate = textureCoordinate;
    gl_Position = proj * view * model * vec4(position, 1.0);
}