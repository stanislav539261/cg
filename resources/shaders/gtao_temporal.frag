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

layout(binding = 0) uniform sampler2D g_AmbientOcclusionSpartialTexture;
layout(binding = 1) uniform sampler2D g_HalfDepthTexture;
layout(binding = 1) uniform sampler2D g_HalfVelocityTexture;
layout(binding = 3) uniform sampler2D g_LastAmbientOcclusionTemporalTexture;
layout(binding = 4) uniform sampler2D g_LastDepthTexture;
layout(location = 0) uniform float g_Rate;
layout(location = 1) uniform vec2 g_ScreenSize;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out float outColor;

float LinearizeZ(const float fDepth, const float fNear, const float fFar) {
	return fNear * fFar / (fDepth * (fNear - fFar) + fFar);
}

void main() {
    const vec2 texcoord = VS_Output.m_Texcoord;
	const float AO = texture(g_AmbientOcclusionSpartialTexture, texcoord).x;
	const vec2 velocity	= texture(g_HalfVelocityTexture, texcoord).xy;
	const vec4 lastAO4 = textureGather(g_LastAmbientOcclusionTemporalTexture, texcoord - velocity);
	const vec4 lastDepth4 = textureGather(g_LastDepthTexture, texcoord - velocity);
	const vec2 offset = (texcoord - velocity) * g_ScreenSize - vec2(0.5f) + vec2(1.f / 512.f);
	const float weightX = 1.f - fract(offset.x);
	const float weightY = fract(offset.y);

	const float depth = texture(g_HalfDepthTexture, texcoord).r;
	const float linearDepth = LinearizeZ(depth, g_FarZ, g_NearZ);

	float weight = 0.f;
	float aoWeighted = 0.f;
	float bilinearWeights[] = float[](
		weightX * weightY,
		(1.f - weightX) * weightY,
		(1.f - weightX) * (1.f - weightY),
		weightX * (1.f - weightY)
	);

	for (int i = 0; i < 4; i++) {
		const float linearLastDepth = LinearizeZ(lastDepth4[i], g_FarZ, g_NearZ);
		const float bilateralWeight = clamp(1.f + 0.1f * (-linearDepth + linearLastDepth), 0.01f, 1.f);

		weight += bilinearWeights[i] * bilateralWeight;
		aoWeighted += bilinearWeights[i] * bilateralWeight * lastAO4[i];
	}

	aoWeighted /= weight;

	const vec2 lastTexcoord = abs(texcoord - velocity - vec2(0.5));

	float rate = g_Rate;

	if (lastTexcoord.x > 0.5 || lastTexcoord.y > 0.5) {
		rate = 1.f;
	}

	float lastDepth;

	if (fract(offset.x) > 0.5f) {
		lastDepth = fract(offset.y) > 0.5f ? lastDepth4.y : lastDepth4.z;
	} else {
		lastDepth = fract(offset.y) > 0.5f ? lastDepth4.x : lastDepth4.w;
	}

	const float lastLinearDepth = LinearizeZ(lastDepth, g_FarZ, g_NearZ);

	// discard history if current fragment was occluded in previous frame
	// also helps with ghosting
	if ((-lastLinearDepth * 0.9f) > lastDepth) {
		rate = 1.f;
	}

	outColor = mix(aoWeighted, AO, g_Rate);
}