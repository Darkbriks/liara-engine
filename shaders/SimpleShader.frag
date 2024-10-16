#version 450

layout (location = 0) out vec4 fragColor; // Cette ligne permet de définir la sortie du shader
void main()
{
    fragColor = vec4(1.0, 0.0, 0.0, 1.0); // Cette ligne définit la couleur de sortie du shader (rouge en l'occurence)
}