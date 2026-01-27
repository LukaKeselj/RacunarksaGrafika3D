#version 330 core
in vec2 TexCoord;
in vec2 FragPos;

out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    float distance = length(FragPos);
    if (distance > 1.0) {
        discard;
    }
    
    vec4 texColor = texture(uTexture, TexCoord);
    FragColor = texColor;
}
