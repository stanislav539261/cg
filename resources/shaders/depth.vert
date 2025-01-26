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

layout(std430, binding = 2) readonly buffer VertexBuffer {
    float g_Vertices[][9];
};

void main() {
    const uint vertex = g_Indices[gl_VertexID];
    const vec3 fragPos = vec3(g_Vertices[vertex][0], g_Vertices[vertex][1], g_Vertices[vertex][2]);

    gl_Position = g_Projection * g_View * vec4(fragPos, 1.f);
}