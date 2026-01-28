#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 uLightColor;
uniform float uIntensity;

void main()
{
    // Emissive material - lampa emituje svetlost
    vec3 emission = uLightColor * uIntensity;
    
    // Dodaj blagi glow efekat na ivicama
    vec3 viewDir = normalize(-FragPos);
    float edgeGlow = 1.0 - abs(dot(Normal, viewDir));
    edgeGlow = pow(edgeGlow, 2.0) * 0.3;
    
    emission += edgeGlow * uLightColor;
    
    FragColor = vec4(emission, 1.0);
}
