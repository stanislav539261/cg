#version 460 core

layout(std430, binding = 0) readonly buffer CameraBuffer {
    mat4  g_LastView;
    mat4  g_Projection;
    mat4  g_ProjectionInversed;
    mat4  g_ProjectionNonReversed;
    mat4  g_ProjectionNonReversedInversed;
    mat4  g_View;
    vec3  g_CameraPos;
    float m_Padding0;
    vec2  g_NormTileDim;
    vec2  g_TileSizeInv;
    float g_FarZ;
    float g_NearZ;
    float g_FovX;
    float g_FovY;
    float g_SliceBiasFactor;
    float g_SliceScalingFactor;
    float m_Padding1;
    float m_Padding2;
};

layout(binding = 0) uniform sampler2D g_AmbientOcclusionSpartialTexture;
layout(binding = 1) uniform sampler2D g_DepthTexture;
layout(binding = 2) uniform sampler2D g_LastAmbientOcclusionTemporalTexture;
layout(binding = 3) uniform sampler2D g_LastDepthTexture;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out float outColor;

vec3 ReconstructViewPos(vec3 srcPos) {
    const vec4 pos = g_ProjectionInversed * vec4(srcPos.xy * 2.f - 1.f, srcPos.z, 1.f);
    return pos.xyz / pos.w;
}

void main() {
    const float currentDepth = texture(g_DepthTexture, VS_Output.m_Texcoord).r;
    const vec3 currentViewPos = ReconstructViewPos(vec3(VS_Output.m_Texcoord, currentDepth));
    const vec4 currentPos = inverse(g_View) * vec4(currentViewPos, 1.f);

    const vec4 lastScrPos4 = g_Projection * g_LastView * currentPos;
    const vec3 lastScrPos = lastScrPos4.xyz / lastScrPos4.w;
    const vec2 lastTexcoord = lastScrPos.xy * 0.5f + vec2(0.5f);
    const float lastDepth = texture(g_LastDepthTexture, lastTexcoord).r;
    const vec3 lastViewPos = ReconstructViewPos(vec3(lastTexcoord, lastDepth));

    float totalAo = 0.f;

    const float currentAo = texture(g_AmbientOcclusionSpartialTexture, VS_Output.m_Texcoord).r;
    const float lastAo = texture(g_LastAmbientOcclusionTemporalTexture, lastTexcoord).r;

    outColor = mix(lastAo, currentAo, clamp(distance(currentViewPos, lastViewPos), 0.f, 1.f));
}