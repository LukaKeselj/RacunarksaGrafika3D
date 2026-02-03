#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uWallColor;
uniform sampler2D uWallTexture;
uniform bool uUseTexture;

void main()
{
    // POVE?AN ambient za svjetliju sobu
    vec3 ambient = 0.4 * uWallColor;  // Bilo 0.25, sada 0.4
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uWallColor * 0.8;  // Poja?an diffuse sa 0.7 na 0.8
    
    // Specular lighting
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = 0.2 * spec * vec3(1.0);
    
    // Soft shadows - BLAŽIJA attenuation za svjetliju sobu
    float distance = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.035 * distance + 0.0044 * distance * distance); // Smanjena attenuation
    
    // Contact shadows - SMANJEN za manje tamne ivice
    float contactShadow = 0.0;
    if (abs(Normal.y) > 0.9) {
        contactShadow = 0.05; // Bilo 0.1, sada 0.05
    }
    
    vec3 baseColor = uWallColor;
    
    if (uUseTexture) {
        vec4 texColor = texture(uWallTexture, TexCoords);
        baseColor = texColor.rgb * uWallColor;
        
        diffuse *= attenuation;
        specular *= attenuation;
        
        vec3 result = (ambient + diffuse) * baseColor + specular;
        result *= (1.0 - contactShadow);
        FragColor = vec4(result, 1.0);
    } else {
        if (abs(Normal.y) > 0.9) {
            float gridSize = 2.0;
            float gridWidth = 0.05;
            
            vec2 gridPos = fract(FragPos.xz / gridSize);
            float grid = step(gridWidth, gridPos.x) * step(gridWidth, gridPos.y) * 
                         step(gridPos.x, 1.0 - gridWidth) * step(gridPos.y, 1.0 - gridWidth);
            
            vec3 gridColor = mix(baseColor * 0.6, baseColor, grid); // Svjetliji grid (bilo 0.4, sada 0.6)
            
            diffuse *= attenuation;
            specular *= attenuation;
            
            vec3 result = (ambient + diffuse) * gridColor + specular;
            result *= (1.0 - contactShadow);
            FragColor = vec4(result, 1.0);
        } else {
            diffuse *= attenuation;
            specular *= attenuation;
            
            vec3 result = (ambient + diffuse) * baseColor + specular;
            FragColor = vec4(result, 1.0);
        }
    }
}
