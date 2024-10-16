#version 450

// Hardcoding triangle vertices for testing
const vec2 vertices[3] = vec2[3](
    vec2(0.0, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5)
);

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0); // Cette ligne permet de définir la position du sommet
    // Le premier paramètre est la position du sommet, le deuxième est la profondeur du sommet (0.0 = proche, 1.0 = loin), et le dernier est la coordonnée homogène (1.0 = point, 0.0 = vecteur)
}