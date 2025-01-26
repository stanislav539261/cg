#version 460 core

layout(std430, binding = 0) readonly buffer CameraBuffer {
    mat4  g_Projection;
    mat4  g_ProjectionInversed;
    mat4  g_View;
    vec3  g_CameraPos;
    float m_Padding0;
    float g_FarZ;
    float g_NearZ;
    float m_Padding1;
    float m_Padding2;
};

layout(std430, binding = 1) readonly buffer IndexBuffer {
    uint g_Indices[];
};

layout(std430, binding = 4) readonly buffer VertexBuffer {
    float g_Vertices[][9];
};

out VS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) smooth vec2 m_Texcoord;
    layout(location = 2) smooth vec3 m_Normal;
    layout(location = 3) flat uint m_Material;
} VS_Output;

void main() {
    const uint vertex = g_Indices[gl_VertexID];
    const vec3 fragPos = vec3(g_Vertices[vertex][0], g_Vertices[vertex][1], g_Vertices[vertex][2]);

    VS_Output.m_FragPos = fragPos.xyz; 
    VS_Output.m_Texcoord = vec2(g_Vertices[vertex][3], g_Vertices[vertex][4]); 
    VS_Output.m_Normal = vec3(g_Vertices[vertex][5], g_Vertices[vertex][6], g_Vertices[vertex][7]); 
    VS_Output.m_Material = floatBitsToUint(g_Vertices[vertex][8]);

    gl_Position = g_Projection * g_View * vec4(fragPos, 1.f);
}