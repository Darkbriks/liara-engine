#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projectionView;
    vec3 lightDirection;
} ubo;

layout(push_constant) uniform PushConstant
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} pushConstant;

const float AMBIENT_INTENSITY = 0.15;

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = ubo.projectionView * pushConstant.modelMatrix * vec4(position, 1.0);

    vec3 normalInWorld = normalize(mat3(pushConstant.normalMatrix) * normal);

    float lightIntensity = AMBIENT_INTENSITY + max(dot(normalInWorld, ubo.lightDirection), 0);
    fragColor = lightIntensity * color;
}