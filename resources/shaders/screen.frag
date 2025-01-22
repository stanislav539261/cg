#version 460 core

layout(binding = 0) uniform sampler2D g_ScreenTexture;

layout(location = 0) out vec4 outColor;

void main() {
    const ivec2 texcoord = ivec2(gl_FragCoord.xy);
    const vec4 screen = texelFetch(g_ScreenTexture, texcoord, 0);

    outColor = vec4(screen.rgb, 1.f);
}