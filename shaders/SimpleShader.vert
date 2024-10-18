#version 450

layout(location = 0) in vec2 vertices;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = vec4(vertices, 0.0, 1.0); // Cette ligne permet de définir la position du sommet
    fragColor = color; // Cette ligne permet de définir la couleur du sommet
}