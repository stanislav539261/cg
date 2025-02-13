#version 460 core

layout(binding = 0) uniform sampler2D g_ScreenTexture;
layout(location = 0) uniform uint g_Type;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out vec4 outColor;

const uint AMBIENT_OCCLUSION = 1 << 0;
const uint LIGHTING = 1 << 1;

void main() {
    const vec4 screen = texture(g_ScreenTexture, VS_Output.m_Texcoord);

    switch (g_Type) {
        case AMBIENT_OCCLUSION:
            outColor = vec4(screen.r, screen.r, screen.r, 1.f);
            break;
        case LIGHTING:
            outColor = vec4(screen.rgb, 1.f);
            break;
        default:
            break;
    }
}