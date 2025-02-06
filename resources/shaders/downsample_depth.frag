#version 460 core

layout(binding = 0) uniform sampler2D g_DepthTexture;
layout(location = 0) uniform bool g_EnableReverseZ;

void main() {
	const ivec2 minTexcoord = ivec2(gl_FragCoord.xy);
    const ivec2 maxTexcoord = minTexcoord * 2;
    const float depth00 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(0, 0), 0).r;
    const float depth01 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(0, 1), 0).r;
    const float depth10 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(1, 0), 0).r;
    const float depth11 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(1, 1), 0).r;
    
    if (g_EnableReverseZ) {
        float depth = min(min(depth00, depth01), min(depth10, depth11));
        bvec2 odd = notEqual(textureSize(g_DepthTexture, 0) & ivec2(1), ivec2(0));

        if (odd.x) {
            float depth20 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(2, 0), 0).r;
            float depth21 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(2, 1), 0).r;

            depth = min(depth, min(depth20, depth21));
        }

        if (odd.y) {
            float depth02 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(0, 2), 0).r;
            float depth12 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(1, 2), 0).r;

            depth = min(depth, min(depth02, depth12));
        }

        if (odd.x && odd.y) {
            float depth22 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(2, 2), 0).r;

            depth = min(depth, depth22);
        }

        gl_FragDepth = depth;
    } else {
        float depth = max(max(depth00, depth01), max(depth10, depth11));
        bvec2 odd = notEqual(textureSize(g_DepthTexture, 0) & ivec2(1), ivec2(0));

        if (odd.x) {
            float depth20 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(2, 0), 0).r;
            float depth21 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(2, 1), 0).r;

            depth = max(depth, max(depth20, depth21));
        }
        
        if (odd.y) {
            float depth02 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(0, 2), 0).r;
            float depth12 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(1, 2), 0).r;

            depth = max(depth, max(depth02, depth12));
        }

        if (odd.x && odd.y) {
            float depth22 = texelFetch(g_DepthTexture, maxTexcoord + ivec2(2, 2), 0).r;

            depth = max(depth, depth22);
        }

        gl_FragDepth = depth;
    }
}