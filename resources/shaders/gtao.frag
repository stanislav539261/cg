#version 460 core

#define M_PI 3.1415926535897932384626433832795f

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

layout(binding = 0) uniform sampler2D g_DepthTexture;
layout(location = 0) uniform float g_FalloffFar;
layout(location = 1) uniform float g_FalloffNear;
layout(location = 2) uniform int g_NumSamples;
layout(location = 3) uniform float g_Offset;
layout(location = 4) uniform float g_Radius;
layout(location = 5) uniform float g_Rotation;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out float outColor;

vec3 ReconstructViewPos(vec3 srcPos) {
	const vec4 pos = g_ProjectionInversed * vec4(srcPos.xy * 2.f - 1.f, srcPos.z, 1);
	return pos.xyz / pos.w;
}

vec3 ReconstructNormal(vec3 srcPos, vec3 viewPos) {
	const vec2 size = textureSize(g_DepthTexture, 0);
	const vec2 up = vec2(0.f, 1.f / size.y);
	const vec2 right = vec2(1.f / size.x, 0.f);
	const float depthUp = textureLod(g_DepthTexture, srcPos.xy + up, 0).r;
	const float depthDown = textureLod(g_DepthTexture, srcPos.xy - up, 0).r;
	const float depthRight = textureLod(g_DepthTexture, srcPos.xy + right, 0).r;
	const float depthLeft = textureLod(g_DepthTexture, srcPos.xy - right, 0).r;
	const bool isUpCloser = abs(depthUp - srcPos.z) < abs(depthDown - srcPos.z);
	const bool isRightCloser = abs(depthRight - srcPos.z) < abs(depthLeft - srcPos.z);

	vec3 p0;
	vec3 p1;

	if (isUpCloser && isRightCloser) {
		p0 = vec3(srcPos.xy + right, depthRight);
		p1 = vec3(srcPos.xy + up, depthUp);
	} else if (isUpCloser && !isRightCloser) {
		p0 = vec3(srcPos.xy + up, depthUp);
		p1 = vec3(srcPos.xy - right, depthLeft);
	} else if (!isUpCloser && isRightCloser) {
		p0 = vec3(srcPos.xy - up, depthDown);
		p1 = vec3(srcPos.xy + right, depthRight);
	} else {
		p0 = vec3(srcPos.xy - right, depthLeft);
		p1 = vec3(srcPos.xy - up, depthDown);
	}

	return -normalize(cross(ReconstructViewPos(p1) - viewPos, ReconstructViewPos(p0)  - viewPos));
}

void main() {
	const vec2 texcoord = VS_Output.m_Texcoord;
	const float depth = textureLod(g_DepthTexture, texcoord, 0).r;
	const vec3 viewPos = ReconstructViewPos(vec3(texcoord, depth));
	const vec3 viewDir = normalize(-viewPos);

	const vec3 normal = ReconstructNormal(vec3(texcoord, depth), viewPos);
	const float radius = g_Radius / abs(viewPos.z);
	const ivec2 xy = ivec2(gl_FragCoord);
	const float rotationNoise = 1.f / 16.f * ((((xy.x + xy.y) & 3) << 2) + (xy.x & 3));
	const float angle = g_Rotation + rotationNoise * M_PI * 2.f;

	const vec3 aoDir = vec3(cos(angle), sin(angle), 0.f);
	const float sampleSize = radius * 1.f / g_NumSamples;
	// const float offsetNoise = 1.f / 4.f * ((((xy.x + xy.y) & 3) << 2) + (xy.x & 3));

	float currentSampleSize = sampleSize;
	vec2 horizons = vec2(-1.f);

	// calculate horizon angles	
	for (uint i = 0; i < g_NumSamples; i++) {
		vec2 offset = aoDir.xy * currentSampleSize;

		for (uint j = 0; j < 2; j++) {			
			// const float level = ceil(log2(max(offset.x * gl_FragCoord.x, offset.y * gl_FragCoord.y)));
			const float depth = textureLod(g_DepthTexture, texcoord + offset, 1).r;
			const vec3 sampleViewPos = ReconstructViewPos(vec3(texcoord + offset, depth));
			const vec3 dir = sampleViewPos - viewPos;
			const float dist = length(dir);
			const float dist2 = dist * dist;
			const float cosh = dot(dir, viewDir) / dist;
			const float falloff = 2.f * (dist2 - g_FalloffNear) / (g_FalloffFar - g_FalloffNear);

			horizons[j] = max(horizons[j], cosh - falloff);
			offset *= -1.f;
		}

		currentSampleSize += sampleSize;
	}

	horizons = acos(horizons);

	// calculate gamma angle
	const vec3 bitangent = normalize(cross(aoDir, viewDir));
	const vec3 tangent = cross(viewDir, bitangent);
	const vec3 nx = normal - bitangent * dot(normal, bitangent);
	const float nnx = length(nx);
	const float invnnx = 1.f / (nnx + 0.000001f);
	const float cosxi = dot(nx, tangent) * invnnx;
	const float gamma = acos(cosxi) - M_PI * 0.5f;
	const float cosgamma = dot(nx, viewDir) * invnnx;
	const float singamma2 = -2.f * cosxi;

	// clamp to normal hemisphere
	horizons.x = gamma + max(-horizons.x - gamma, M_PI * -0.5f);
	horizons.y = gamma + min( horizons.y - gamma, M_PI *  0.5f);

	// Riemann integral is additive
	float ao = 0.f;
	ao += horizons.x * singamma2 + cosgamma - cos(2.f * horizons.x - gamma);
	ao += horizons.y * singamma2 + cosgamma - cos(2.f * horizons.y - gamma);
	ao *= nnx * 0.25f;

	outColor = ao;
}