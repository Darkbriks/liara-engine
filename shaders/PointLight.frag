#version 450

layout(location = 0) in vec2 fragOffset;
layout(location = 0) out vec4 outColor;

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

layout(push_constant) uniform Push
{
    vec4 position;
    vec4 color;
    float radius;
} push;

void main()
{
    // Discard fragments outside the light sphere
    float squareDistance = dot(fragOffset, fragOffset);
    if (squareDistance > 1.0) { discard; }

    outColor = vec4(push.color.xyz, 1.0);
}