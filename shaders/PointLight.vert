#version 450

const vec2 OFFSETS[6] = vec2[](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;

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

layout(push_constant) uniform Push
{
    vec4 position;
    vec4 color;
    float radius;
} push;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = vec3(ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]);
    vec3 cameraUpWorld = vec3(ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]);

    vec3 positionWorld = push.position.xyz
        + cameraRightWorld * fragOffset.x * push.radius
        + cameraUpWorld * fragOffset.y * push.radius;

    gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}