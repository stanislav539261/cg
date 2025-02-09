#version 460 core

layout(std430, binding = 0) readonly buffer CameraBuffer {
    mat4  g_LastProjection;
    mat4  g_LastProjectionInversed;
    mat4  g_LastView;
    mat4  g_Projection;
    mat4  g_ProjectionInversed;
    mat4  g_View;
    vec3  g_CameraPos;
    float m_Padding0;
    float g_FarZ;
    float g_NearZ;
    float g_FovX;
    float g_FovY;
};

layout(binding = 0) uniform sampler2D g_AmbientOcclusionTexture;
layout(binding = 1) uniform sampler2D g_DepthTexture;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out float outColor;

float LinearizeZ(const float fDepth, const float fNear, const float fFar) {
	return fNear * fFar / (fDepth * (fNear - fFar) + fFar);
}

void main() {
    const vec2 size = textureSize(g_AmbientOcclusionTexture, 0);
	const vec2 offset = 1.f / size * 2.f;
    const vec2 texcoord = VS_Output.m_Texcoord - offset;

	vec4 ambientOcclusion4x4[4];
	vec4 depth4x4[4];

	ambientOcclusion4x4[0] = textureGatherOffset(g_AmbientOcclusionTexture, texcoord, ivec2(0, 0));
	ambientOcclusion4x4[1] = textureGatherOffset(g_AmbientOcclusionTexture, texcoord, ivec2(0, 2));
	ambientOcclusion4x4[2] = textureGatherOffset(g_AmbientOcclusionTexture, texcoord, ivec2(2, 0));
	ambientOcclusion4x4[3] = textureGatherOffset(g_AmbientOcclusionTexture, texcoord, ivec2(2, 2));
	depth4x4[0] = textureGatherOffset(g_DepthTexture, texcoord, ivec2(0, 0));
	depth4x4[1] = textureGatherOffset(g_DepthTexture, texcoord, ivec2(0, 2));
	depth4x4[2] = textureGatherOffset(g_DepthTexture, texcoord, ivec2(2, 0));
	depth4x4[3] = textureGatherOffset(g_DepthTexture, texcoord, ivec2(2, 2));
	
	const float depth = LinearizeZ(depth4x4[0].x, g_FarZ, g_NearZ);
	const float threshold = abs(0.1f * depth);

	float totalAo = 0.f;
	float totalWeight = 0.f;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			const float diff = abs(LinearizeZ(depth4x4[i][j], g_FarZ, g_NearZ) - depth);

			if (diff < threshold) {
				const float weight = 1.f - clamp(10.f * diff / threshold, 0.f, 1.f);

				totalAo += ambientOcclusion4x4[i][j] * weight;
				totalWeight += weight;
			}
		}
	}

	outColor = totalAo * 1.f / totalWeight;
}