#version 460 core

in VS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) flat vec3 m_LightPos;
    layout(location = 2) flat float m_Radius;
} VS_Output;

layout(location = 0) out float outColor;

void main() {
    const float depth = distance(VS_Output.m_LightPos, VS_Output.m_FragPos) * 1.f / VS_Output.m_Radius;
    const float dx = dFdx(depth);
    const float dy = dFdy(depth);

    outColor = depth * depth + 0.25f * (dx * dx + dy * dy);
    gl_FragDepth = depth;
}