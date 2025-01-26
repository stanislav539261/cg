#version 460 core

layout(binding = 0) uniform sampler2D g_DepthTexture;
layout(binding = 0) uniform sampler2D g_VelocityTexture;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out float outDepth;
layout(location = 1) out vec2 outVelocity;

void main() {
    const vec2 texcoord = VS_Output.m_Texcoord;
	const vec4 depth4 = textureGather(g_DepthTexture, texcoord);
	const vec4 velX4 = textureGather(g_VelocityTexture, texcoord, 0);
	const vec4 velY4 = textureGather(g_VelocityTexture, texcoord, 1);

	float depth = depth4[0];
	vec2 velocity = vec2(velX4[0], velY4[0]);

	for (int i = 1; i < 4; i++) {
		if (depth4[i] < depth) {
			depth = depth4[i];
			velocity = vec2(velX4[i], velY4[i]);
		}
	}

	outDepth = depth;
	outVelocity = velocity;
}