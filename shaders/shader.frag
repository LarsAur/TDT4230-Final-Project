#version 450 core

in vec3 fragWorldPos;
in vec3 fragNormal;
in vec2 fragTextureCoordinate;

#define N_LIGHTS 5
uniform vec3 u_light_positions[N_LIGHTS];
uniform vec3 u_light_colors[N_LIGHTS];
uniform vec3 u_camera_position;
uniform int u_is_portal = 0;
uniform vec3 u_portal_color;
uniform float u_time;

layout(binding = 0) uniform sampler2D texDiffuse;
layout(binding = 1) uniform sampler2D noise;

out vec4 color;

const float alpha = 32.0;
const float ambient = 0.3;
const float spec_factor = 0.5;
const float diff_factor = 1.0;

void main()
{

    if(u_is_portal == 0)
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

            float attenuation = 1 / (0.01 + 0.05*dist + 0.01*pow(dist, 2));
            
            diffuse += attenuation * color * clamp(dot(normalize(L_m), fragNormal), 0, 1);
            specular += attenuation * color * pow(clamp(dot(normalize(R_m), normalize(V)), 0, 1), alpha) ;
        }

        color = (vec4(vec3(ambient), 1) + vec4(diffuse * diff_factor, 1))  * texture(texDiffuse, fragTextureCoordinate) + vec4(specular * spec_factor, 0); 
    }
    else
    {
        // Estimating an eliptic border
        // https://stackoverflow.com/questions/51384738/draw-a-ellipse-curve-in-fragment-shader
        mat2 rotation;
        rotation[0] = vec2(cos(u_time), -sin(u_time));
        rotation[1] = vec2(sin(u_time), cos(u_time));
        const float border = 1.0 - texture(noise, rotation * (fragTextureCoordinate - 0.5)*2).r;
        const float width = 5;
        const float height = 10;

        float _x = (fragTextureCoordinate.x - 0.5) * 2;
        float _y = (fragTextureCoordinate.y - 0.5) * 2;
        float radius = sqrt(pow(_x, 2) + pow(_y, 2));

        float rH = 1.0 - border / width;
        float rV = 1.0 - border / height;
        float radiusAvg = (rH + rV) / 2;

        float minRadius = 0.0;
        _x = abs(_x);
        _y = abs(_y);
        if(_x>_y) {
            minRadius = mix( rH, radiusAvg, _y / _x );
        }
        else {
            minRadius = mix( rV, radiusAvg, _x / _y );
        }

        color = vec4(0);
        if(radius > minRadius)
        {
            color = vec4(u_portal_color - texture(noise, (-rotation * (fragTextureCoordinate - 0.5)) + 0.5).r * 0.5, 1.0);
        }
    }
}