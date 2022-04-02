#version 450 core

in vec3 fragWorldPos;
in vec3 fragNormal;
in vec2 fragTextureCoordinate;

#define N_LIGHTS 1
uniform vec3 u_light_positions[N_LIGHTS];
uniform vec3 u_light_colors[N_LIGHTS];
uniform vec3 u_camera_position;
uniform int u_is_portal = 0;

layout(binding = 0) uniform sampler2D texDiffuse;

const float alpha = 32.0;

void main()
{
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
    vec3 V = u_camera_position - fragWorldPos;
    for(int i = 0; i < N_LIGHTS; i++)
    {
        vec3 lightPos = u_light_positions[i];
        vec3 color = u_light_colors[i];
        
        vec3 L_m = lightPos - fragWorldPos;
        vec3 R_m = reflect(-normalize(L_m), fragNormal);

        float dist = length(L_m);
        
        diffuse += color * clamp(dot(normalize(L_m), fragNormal), 0, 1);
        specular += color * pow(clamp(dot(normalize(R_m), normalize(V)), 0, 1), alpha) ;
    }

    gl_FragColor = vec4(diffuse, 1) * texture(texDiffuse, fragTextureCoordinate) + vec4(specular, 0); 

}