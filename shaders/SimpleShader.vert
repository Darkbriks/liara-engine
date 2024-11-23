#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform PushConstant
{
    mat4 transform;
    mat4 normalMatrix;
} pushConstant;

const vec3 LIGHT_DIRECTION = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT_INTENSITY = 0.05;

// La fonction main est exécutée une fois par sommet
void main()
{
    gl_Position = pushConstant.transform * vec4(position, 1.0);

    vec3 normalInWorld = normalize(mat3(pushConstant.normalMatrix) * normal);

    float lightIntensity = AMBIENT_INTENSITY + max(dot(normalInWorld, LIGHT_DIRECTION), 0);
    fragColor = lightIntensity * color;
}