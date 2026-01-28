#version 330 core

layout(location = 0) in vec2 aPos;

void main()
{
    // Direktne NDC koordinate - BEZ projection matrix-a!
    gl_Position = vec4(aPos, 0.0, 1.0);
}
