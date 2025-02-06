#version 460 core

#define M_PI 3.1415926535897932384626433832795f

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
layout(location = 0) uniform vec2 g_ScreenSize;
layout(location = 1) uniform vec2 g_ScreenSizeInv;
layout(location = 2) uniform float g_Radius;
layout(location = 3) uniform float g_Rotation;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out float outColor;

vec3 ReconstructPosition(float depth, vec2 texcoord) {
	const vec4 pos = g_ProjectionInversed * vec4(texcoord * 2 - 1, depth, 1);
	return pos.xyz / pos.w;
}

vec3 ReconstructNormal(vec2 texcoord, float depthCenter, vec3 center) {
	vec2 up = vec2(0.f, g_ScreenSizeInv.y);
	vec2 right = vec2(g_ScreenSizeInv.x, 0.f);
	float depthUp = texture(g_DepthTexture, texcoord + up).r;
	float depthDown = texture(g_DepthTexture, texcoord - up).r;
	float depthRight = texture(g_DepthTexture, texcoord + right).r;
	float depthLeft = texture(g_DepthTexture, texcoord - right).r;
	bool isUpCloser = abs(depthUp - depthCenter) < abs(depthDown - depthCenter);
	bool isRightCloser = abs(depthRight - depthCenter) < abs(depthLeft - depthCenter);

	vec3 p0;
	vec3 p1;

	if (isUpCloser && isRightCloser) {
		p0 = vec3(texcoord + right, depthRight);
		p1 = vec3(texcoord + up, depthUp);
	} else if (isUpCloser && !isRightCloser) {
		p0 = vec3(texcoord + up, depthUp);
		p1 = vec3(texcoord - right, depthLeft);
	} else if (!isUpCloser && isRightCloser) {
		p0 = vec3(texcoord - up, depthDown);
		p1 = vec3(texcoord + right, depthRight);
	} else {
		p0 = vec3(texcoord - right, depthLeft);
		p1 = vec3(texcoord - up, depthDown);
	}

	return -normalize(cross(ReconstructPosition(p1.z, p1.xy) - center, ReconstructPosition(p0.z, p0.xy)  - center));
}

void main() {
    const vec2 texcoord = VS_Output.m_Texcoord;
	const float depth = textureLod(g_DepthTexture, texcoord, 0).r;
	const vec3 center = ReconstructPosition(depth, texcoord);
	const vec3 v = normalize(-center);
	const vec3 normal = ReconstructNormal(texcoord, depth, center);
	const float radius = min(g_Radius / abs(center.z), g_Radius);

	const ivec2 xy = ivec2(gl_FragCoord);
	const float radAngle = g_Rotation + (1.f / 16.f) * ((((xy.x + xy.y) & 3) << 2) + (xy.x & 3)) * M_PI * 2.f;

	const vec3 direction = vec3(cos(radAngle), sin(radAngle), 0);
	const vec3 orthoDirection = direction - dot(direction, v) * v;
	const vec3 axis = cross(direction, v);
	const vec3 projectedNormal = normal - dot(normal, axis) * axis;

	const float signN = sign(dot(orthoDirection, projectedNormal));
	const float cosN = clamp(dot(projectedNormal, v) / length(projectedNormal), 0, 1);
	const float n = signN * acos(cosN);

	const int kNumDirectionSamples = 6;
	const float stepSize = radius / kNumDirectionSamples;
	float ao = 0;

	for (int side = 0; side < 2; side++) {
		float horizonCos = -1.f;
		vec2 offset = (-1.f + 2.f * side) * direction.xy * 0.25f * ((xy.y - xy.x) & 3) * g_ScreenSizeInv;

		for (int i = 0; i < kNumDirectionSamples; i++) {
			offset += (-1.f + 2.f * side) * direction.xy * (stepSize);

			const float lod = floor(log2(max(texcoord.x + offset.x, texcoord.y + offset.y) * 0.5f));
			const float depth = textureLod(g_DepthTexture, texcoord + offset, lod).r;
			const vec3 samplePos = ReconstructPosition(depth, texcoord + offset);
			const vec3 horizonPos = samplePos - center;
			const float horizonLength = length(horizonPos);
            
			if (horizonLength < 56.89 / 4.f) {
				horizonCos = max(horizonCos, dot(horizonPos, v) / horizonLength);
            }
		}
		const float radHorizon = n + clamp((-1.f + 2.f * side) * acos(horizonCos) - n, -M_PI * 0.5f, M_PI * 0.5f);

		ao += length(projectedNormal) * 0.25 * (cosN + 2.f * radHorizon * sin(n) - cos(2.f * radHorizon -n));
	}

	outColor = ao;
}