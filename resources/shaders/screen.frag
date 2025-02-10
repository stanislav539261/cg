#version 460 core

layout(binding = 0) uniform sampler2D g_ScreenTexture;

in VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

layout(location = 0) out vec4 outColor;

void main() {
    const vec4 screen = texture(g_ScreenTexture, VS_Output.m_Texcoord);

    outColor = vec4(screen.rgb, 1.f);
}