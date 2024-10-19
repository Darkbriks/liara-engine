#version 450

layout (location = 0) out vec4 fragColor; // Cette ligne permet de définir la sortie du shader

layout(push_constant) uniform PushConstant {
    vec2 offset;
    vec3 color;
} pushConstant;

void main()
{
    fragColor = vec4(pushConstant.color, 1.0); // Cette ligne permet de définir la couleur de chaque pixel
}