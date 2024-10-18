#version 450

layout (location = 0) in vec3 color; // Cette ligne permet de définir l'entrée du shader
layout (location = 0) out vec4 fragColor; // Cette ligne permet de définir la sortie du shader

void main()
{
    fragColor = vec4(color, 1.0); // Cette ligne permet de définir la couleur de chaque pixel
}