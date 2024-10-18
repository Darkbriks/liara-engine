#version 450

layout(location = 0) in vec2 vertices; // Cette ligne permet de définir les coordonnées du sommet

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = vec4(vertices, 0.0, 1.0); // Cette ligne permet de définir la position du sommet
}