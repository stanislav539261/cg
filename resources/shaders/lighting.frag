#version 460 core

struct Material {
    uint m_DiffuseMap;
    uint m_MetalnessMap;
    uint m_NormalMap;
    uint m_RoughnessMap;
};

layout(std430, binding = 2) readonly buffer MaterialBuffer {
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

    outColor = vec4(texture(g_DiffuseTextures, vec3(VS_Output.m_Texcoord, diffuseMap)).rgb, 1.f);
}