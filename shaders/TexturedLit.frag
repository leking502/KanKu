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
layout(set = 2, binding = 0) uniform sampler2D tex1;

const float AMBIENT = 0.02;


void main()
{
    vec3 color = texture(tex1,texCoord).xyz;
    float lightIntensity = AMBIENT + max(dot(inNormalWorld, sceneData.sunlightDirection.xyz),0.2);
    outFragColor = lightIntensity * vec4(color,1.0f);
}