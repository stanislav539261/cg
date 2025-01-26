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

layout(binding = 0) uniform sampler2D g_DepthTexture;
layout(binding = 1) uniform sampler2D g_LastDepthTexture;
layout(location = 0) uniform vec2 g_Jitter;
layout(location = 1) uniform vec2 g_LastJitter;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out vec2 outColor;

vec4 ReconstructPosition(float depth, vec2 texcoord) {
	return g_ProjectionInversed * vec4(texcoord * 2.f - 1.f, depth, 1.f);
}

void main() {
    const vec4 position = ReconstructPosition(texture(g_DepthTexture, VS_Output.m_Texcoord).r, VS_Output.m_Texcoord);
    const vec4 lastPosition = ReconstructPosition(texture(g_LastDepthTexture, VS_Output.m_Texcoord).r, VS_Output.m_Texcoord);

    outColor = ((position.xy / position.w - g_Jitter) - (lastPosition.xy / lastPosition.w - g_LastJitter)) * 0.5f;
}