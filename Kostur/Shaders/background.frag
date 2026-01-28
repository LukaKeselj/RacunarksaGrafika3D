#version 330 core

in vec3 WorldPos;
out vec4 FragColor;

uniform sampler2D uBackgroundTexture;

void main()
{
    // Konvertuj 3D poziciju u 2D texture koordinate
    vec3 dir = normalize(WorldPos);
    
    // Sferna projekcija (panoramski skybox) - ispravljena orijentacija
    float u = 0.5 - atan(dir.z, dir.x) / (2.0 * 3.14159265359);
    float v = 0.5 + asin(dir.y) / 3.14159265359;
    
    // KORISTI SAMO LEVU POLOVINU SLIKE (0.0 - 0.5 range za U)
    // Slika je stereoscopic, pa mapiramo na samo levu polovinu
    u = u * 0.5; // Smanji na pola (samo leva polovina)
    
    // Zoom out effect
    vec2 texCoords = vec2(u, v) * 1.5;
    
    // Sample teksturu sa wrapping
    vec4 texColor = texture(uBackgroundTexture, texCoords);
    
    // Atmosferski efekat - gradijent od horizonta ka nebu
    float horizonBlend = smoothstep(-0.2, 0.3, dir.y);
    vec3 horizonColor = vec3(0.6, 0.7, 0.85); // Svetla plava za horizont
    vec3 skyColor = vec3(0.7, 0.85, 1.0);     // Svetlija plava za nebo
    vec3 atmosphereColor = mix(horizonColor, skyColor, horizonBlend);
    
    // Sun glow effect (simulacija sunca)
    vec3 sunDir = normalize(vec3(0.5, 0.8, -0.3));
    float sunIntensity = pow(max(dot(dir, sunDir), 0.0), 32.0);
    vec3 sunGlow = vec3(1.0, 0.95, 0.8) * sunIntensity * 0.5;
    
    // Vignette effect (tamniji rubovi)
    float vignette = 1.0 - smoothstep(0.5, 1.5, length(dir.xz));
    
    // Blend sve zajedno
    vec3 finalColor = mix(texColor.rgb * 0.75, atmosphereColor, 0.35);
    finalColor += sunGlow;
    finalColor *= vignette * 0.5 + 0.5;
    
    FragColor = vec4(finalColor, 1.0);
}
