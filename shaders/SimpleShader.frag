#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) flat in uint fragSpecularExponent; // TODO : Use a material property instead of this

layout (location = 0) out vec4 outColor;

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
    vec3 ambientLight = ubo.directionalLightColor.xyz * ubo.directionalLightColor.w;
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.inverseView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    vec3 surfaceNormal = normalize(fragNormalWorld);
    vec3 lightDir = normalize(ubo.directionalLightDirection.xyz);
    float diffuseFactor = max(dot(surfaceNormal, lightDir), 0.0);
    vec3 diffuseLight = ubo.directionalLightColor.xyz * diffuseFactor * ubo.directionalLightDirection.w;

    // Specular
    /*vec3 halfAngle = normalize(lightDir + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0.0, 1.0);
    blinnTerm = pow(blinnTerm, fragSpecularExponent); // Higer values -> sharper highlights ; TODO : Use a material property instead of hardcoded value
    specularLight += ubo.directionalLightColor.xyz * blinnTerm;*/

    for (int i = 0; i < ubo.numLights; i++)
    {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // Distance squared
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        // Specular
        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0.0, 1.0);
        blinnTerm = pow(blinnTerm, fragSpecularExponent); // Higer values -> sharper highlights ; TODO : Use a material property instead of hardcoded value
        specularLight += intensity * blinnTerm;
    }

    outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}