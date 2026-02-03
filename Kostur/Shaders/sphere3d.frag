#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform float uTime;

void main()
{
    // Tekstura
    vec4 texColor = texture(uTexture, TexCoords);
    
    // Ako je tekstura providna, odbaci fragment
    if (texColor.a < 0.1)
        discard;
    
    // POVE?AN ambient za svjetliju sobu
    vec3 ambient = 0.5 * vec3(1.0);  // Bilo 0.3, sada 0.5
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    
    // Specular lighting
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Pove?an shininess
    vec3 specular = 0.5 * spec * vec3(1.0);
    
    // Soft shadows - BLAŽIJA attenuation za svjetliju sobu
    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * distance * distance); // Smanjena attenuation
    
    // Pulsating glow effect - samo blagi efekat
    float pulse = 0.5 + 0.5 * sin(uTime * 2.0);
    vec3 glowColor = vec3(1.0, 0.3, 0.3);
    
    // Edge detection za glow - manji intenzitet
    float edgeFactor = 1.0 - abs(dot(norm, viewDir));
    edgeFactor = pow(edgeFactor, 2.0);
    vec3 glow = glowColor * edgeFactor * pulse * 0.15;
    
    // Primijeni attenuation (soft shadows)
    diffuse *= attenuation;
    specular *= attenuation;
    
    // Combine sve
    vec3 result = (ambient + diffuse + specular) * texColor.rgb + glow;
    
    FragColor = vec4(result, 1.0);
}
