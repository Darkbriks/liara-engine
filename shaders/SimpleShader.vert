#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(push_constant) uniform PushConstant
{
    mat2 transform;
    vec2 offset;
    vec3 color;
} pushConstant;

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = vec4(pushConstant.transform * position + pushConstant.offset, 0.0, 1.0); // Cette ligne permet de définir la position du sommet
}