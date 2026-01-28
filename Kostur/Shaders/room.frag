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
    // Ambient lighting
    vec3 ambient = 0.4 * uWallColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uWallColor * 0.6;
    
    // Specular lighting
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    vec3 specular = 0.1 * spec * vec3(1.0);
    
    vec3 baseColor = uWallColor;
    
    // If using texture, apply it
    if (uUseTexture) {
        vec4 texColor = texture(uWallTexture, TexCoords);
        baseColor = texColor.rgb * uWallColor;
        
        // Use texture directly without grid pattern
        vec3 result = (ambient + diffuse) * baseColor + specular;
        FragColor = vec4(result, 1.0);
    } else {
        // Grid pattern only for non-textured floor/ceiling
        if (abs(Normal.y) > 0.9) {
            float gridSize = 2.0;
            float gridWidth = 0.05;
            
            vec2 gridPos = fract(FragPos.xz / gridSize);
            float grid = step(gridWidth, gridPos.x) * step(gridWidth, gridPos.y) * 
                         step(gridPos.x, 1.0 - gridWidth) * step(gridPos.y, 1.0 - gridWidth);
            
            vec3 gridColor = mix(baseColor * 0.5, baseColor, grid);
            vec3 result = (ambient + diffuse) * gridColor + specular;
            FragColor = vec4(result, 1.0);
        } else {
            vec3 result = (ambient + diffuse) * baseColor + specular;
            FragColor = vec4(result, 1.0);
        }
    }
}
