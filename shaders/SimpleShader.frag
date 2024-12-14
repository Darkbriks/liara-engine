#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight
{
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec4 directionalLightDirection; // xyz is direction, w is intensity
    vec4 directionalLightColor; // w is ambient intensity
    PointLight pointLights[10]; // TODO: Use a Specialization Constant
    int numLights;
} ubo;

layout(push_constant) uniform PushConstant
{
    mat4 modelMatrix;
    mat4 normalMatrix;
} pushConstant;

void main()
{
    vec3 ambientLight = ubo.directionalLightColor.xyz * ubo.directionalLightColor.w;

    vec3 surfaceNormal = normalize(fragNormalWorld);
    vec3 lightDir = normalize(ubo.directionalLightDirection.xyz);
    float diffuseFactor = max(dot(surfaceNormal, lightDir), 0.0);
    vec3 diffuseLight = ubo.directionalLightColor.xyz * diffuseFactor * ubo.directionalLightDirection.w;

    for (int i = 0; i < ubo.numLights; i++)
    {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // Distance squared
        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;
    }

    outColor = vec4(fragColor * (ambientLight + diffuseLight), 1.0);
}