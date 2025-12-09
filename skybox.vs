#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view; // Note: This matrix should have translation removed (done in C++)

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Force z = w, which results in z/w = 1.0 (far plane)
}