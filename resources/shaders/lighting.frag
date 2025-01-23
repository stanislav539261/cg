#version 460 core

struct LightEnvironment {
    vec3  m_AmbientColor;
    float m_Padding0;
    vec3  m_BaseColor;
    float m_Padding1;
    vec3  m_Direction;
    float m_Padding2;
};

struct Material {
    uint m_DiffuseMap;
    uint m_MetalnessMap;
    uint m_NormalMap;
    uint m_RoughnessMap;
};

layout(std430, binding = 2) readonly buffer LightEnvironmentBuffer {
    LightEnvironment g_LightEnvironment;
};

layout(std430, binding = 3) readonly buffer MaterialBuffer {
    Material g_Materials[];
};

layout(binding = 0) uniform sampler2DArray g_DiffuseTextures;
layout(binding = 1) uniform sampler2DArray g_MetalnessTextures;
layout(binding = 2) uniform sampler2DArray g_NormalTextures;
layout(binding = 3) uniform sampler2DArray g_RoughnessTextures;

in VS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) smooth vec2 m_Texcoord;
    layout(location = 2) smooth vec3 m_Normal;
    layout(location = 3) flat uint m_Material;
} VS_Output;

layout(location = 0) out vec4 outColor;

void main() {
    uint diffuseMap = g_Materials[VS_Output.m_Material].m_DiffuseMap;
    vec4 diffuseColor = texture(g_DiffuseTextures, vec3(VS_Output.m_Texcoord, diffuseMap));

    vec3 normal = normalize(VS_Output.m_Normal);
    vec3 lightDir = normalize(-g_LightEnvironment.m_Direction);  
    float diff = max(dot(normal, lightDir), 0.f);

    vec3 ambientLighting = g_LightEnvironment.m_AmbientColor * diffuseColor.rgb;
    vec3 diffuseLighting = g_LightEnvironment.m_BaseColor * diffuseColor.rgb * diff;  

    outColor = vec4(ambientLighting + diffuseLighting, 1.f);
}