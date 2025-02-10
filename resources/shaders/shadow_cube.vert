#version 460 core
#extension GL_AMD_vertex_shader_layer : require

struct LightPoint {
    mat4  m_ViewProjections[6];
    vec3  m_Position;
    float m_Radius;
    vec3  m_BaseColor;
    float m_Padding0;
};

layout(std430, binding = 0) readonly buffer IndexBuffer {
    uint g_Indices[];
};

layout(std430, binding = 1) readonly buffer LightPointBuffer {
    LightPoint g_LightPoints[];
};

layout(std430, binding = 2) readonly buffer VertexBuffer {
    float g_Vertices[][9];
};

layout(location = 0) uniform uint g_Layer;
layout(location = 1) uniform uint g_LightIndex;

out VS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) flat vec3 m_LightPos;
    layout(location = 2) flat float m_Radius;
} VS_Output;

void main() {
    const uint vertex = g_Indices[gl_VertexID];
    const vec4 fragPos = vec4(g_Vertices[vertex][0], g_Vertices[vertex][1], g_Vertices[vertex][2], 1.f);

    VS_Output.m_FragPos = fragPos.xyz;
    VS_Output.m_LightPos = g_LightPoints[g_LightIndex].m_Position;
    VS_Output.m_Radius = g_LightPoints[g_LightIndex].m_Radius;

    gl_Layer = int(g_Layer);
    gl_Position = g_LightPoints[g_LightIndex].m_ViewProjections[g_Layer] * fragPos;
}