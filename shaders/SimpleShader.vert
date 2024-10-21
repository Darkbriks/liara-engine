#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstant
{
    mat4 transform;
    vec3 color;
} pushConstant;

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = pushConstant.transform * vec4(position, 1.0);
    fragColor = color;
}