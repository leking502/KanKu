#version 460

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;
layout (location = 3) in vec2 vTexCoord;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 texCoord;
layout (location = 2) out vec3 outNormalWorld;
layout (location = 3) out vec3 outPosWorld;

layout (push_constant) uniform Push {
    mat4 modelMatrix;
    vec4 color;
} push;

layout(set = 0, binding = 0) uniform  CameraBuffer{
    mat4 projectionView;
    mat4 lightDirection;
} cameraData;

struct ObjectData{
    mat4 modelMatrix;
    mat4 normalMatrix;
};

layout(std140,set = 1, binding = 0) readonly buffer ObjectBuffer{
    ObjectData objects[];
} objectBuffer;

void main()
{
    mat4 modelMatrix = objectBuffer.objects[gl_BaseInstance].modelMatrix;
    mat4 normalMatrix = objectBuffer.objects[gl_BaseInstance].normalMatrix;
    vec4 positionWorld = modelMatrix * vec4(vPosition, 1.0f);
    gl_Position = cameraData.projectionView * positionWorld;
    outNormalWorld = normalize(mat3(normalMatrix) * vNormal);
    outPosWorld = positionWorld.xyz;
    outColor = push.color.xyz;
    texCoord = vTexCoord;
}