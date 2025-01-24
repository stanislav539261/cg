#version 460 core

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

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std430, binding = 1) readonly buffer LightEnvironmentBuffer {
    LightEnvironment g_LightEnvironment;
};

void main() {
	for (int i = 0; i < 3; ++i) {
		gl_Position = g_LightEnvironment.m_CascadeViewProjections[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		EmitVertex();
	}

	EndPrimitive();
}