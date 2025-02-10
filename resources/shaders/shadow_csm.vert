#version 460 core
#extension GL_AMD_vertex_shader_layer : require

struct LightEnvironment {
    mat4  m_CascadeViewProjections[5];
    vec4  m_CascadePlaneDistances;
    vec3  m_AmbientColor;
    float m_Padding0;
    vec3  m_BaseColor;
    float m_Padding1;
    vec3  m_Direction;
    float m_Padding2;
};

layout(std430, binding = 0) readonly buffer IndexBuffer {
    uint g_Indices[];
};

layout(std430, binding = 1) readonly buffer LightEnvironmentBuffer {
    LightEnvironment g_LightEnvironment;
};

layout(std430, binding = 2) readonly buffer VertexBuffer {
    float g_Vertices[][9];
};

layout(location = 0) uniform uint g_Cascade;

void main() {
    const uint vertex = g_Indices[gl_VertexID];
    const vec4 fragPos = vec4(g_Vertices[vertex][0], g_Vertices[vertex][1], g_Vertices[vertex][2], 1.f);

    gl_Layer = int(g_Cascade);
    gl_Position = g_LightEnvironment.m_CascadeViewProjections[g_Cascade] * fragPos;
}