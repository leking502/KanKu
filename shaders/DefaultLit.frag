//glsl version 4.5
#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormalWorld;
layout (location = 3) in vec3 inPosWorld;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 1) uniform  SceneData{
    vec4 fogColor; // w is for exponent
    vec4 fogDistances; //x for min, y for max, zw unused.
    vec4 ambientColor;
    vec4 sunlightDirection; //w for sun power
    vec4 sunlightColor;
} sceneData;

void main()
{
    vec3 lightColor = sceneData.sunlightColor.xyz * sceneData.sunlightColor.w;
    vec3 ambientLight = sceneData.ambientColor.xyz * sceneData.ambientColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(inNormalWorld), normalize(sceneData.sunlightDirection.xyz)),0);
    outFragColor = vec4((diffuseLight + ambientLight) * inColor,1.0f);
}