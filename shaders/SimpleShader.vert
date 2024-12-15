#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;
layout(location = 4) in uint specularExponent; // TODO : Use a material property instead of this

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out uint fragSpecularExponent; // TODO : Use a material property instead of this

layout(constant_id = 0) const uint MAX_LIGHTS = 10;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    mat4 inverseView;
    vec4 directionalLightDirection; // xyz is direction, w is intensity
    vec4 directionalLightColor; // w is ambient intensity
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
} ubo;

layout(push_constant) uniform PushConstant
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} pushConstant;

void main()
{
    vec4 positionWorld = pushConstant.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    fragNormalWorld = normalize(mat3(pushConstant.normalMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    fragSpecularExponent = specularExponent;
}