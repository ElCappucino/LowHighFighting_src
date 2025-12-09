#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 colorTint;   // <-- add this

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    FragColor = texColor * vec4(colorTint, 1.0);  // <-- multiply color
}