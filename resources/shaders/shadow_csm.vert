#version 460 core

layout(std430, binding = 0) readonly buffer IndexBuffer {
    uint g_Indices[];
};

layout(std430, binding = 2) readonly buffer VertexBuffer {
    float g_Vertices[][9];
};

void main() {
    const uint vertex = g_Indices[gl_VertexID];
    const vec3 fragPos = vec3(g_Vertices[vertex][0], g_Vertices[vertex][1], g_Vertices[vertex][2]);

    gl_Position = vec4(fragPos, 1.f);
}