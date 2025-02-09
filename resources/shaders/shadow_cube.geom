#version 460 core

struct LightPoint {
    mat4  m_ViewProjections[6];
    vec3  m_Position;
    float m_Radius;
    vec3  m_BaseColor;
    float m_Padding0;
};

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std430, binding = 1) readonly buffer LightPointBuffer {
    LightPoint g_LightPoints[];
};

out GS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) flat vec3 m_LightPos;
    layout(location = 2) flat float m_Radius;
} GS_Output;

layout(location = 0) uniform uint g_LightIndex;

void main() {
    gl_Layer = gl_InvocationID;

    for (uint j = 0; j < 3; j++) {
        const vec4 fragPos = gl_in[j].gl_Position;

        GS_Output.m_FragPos = fragPos.xyz;
        GS_Output.m_LightPos = g_LightPoints[g_LightIndex].m_Position;
        GS_Output.m_Radius = g_LightPoints[g_LightIndex].m_Radius;

        gl_Position = g_LightPoints[g_LightIndex].m_ViewProjections[gl_InvocationID] * fragPos;
        EmitVertex();
    }

    EndPrimitive();
}