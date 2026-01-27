#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec2 FragPos;

uniform mat4 uProjection;
uniform mat4 uModel;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 0.0, 1.0);
    gl_Position = uProjection * worldPos;
    TexCoord = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
    FragPos = aPos;
}
