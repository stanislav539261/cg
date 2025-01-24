#version 460 core

layout(location = 0) out float outColor;

void main() {
    float depth = gl_FragCoord.z;
    float dx = dFdx(depth);
    float dy = dFdy(depth);

    outColor = depth * depth + 0.25f * (dx * dx + dy * dy);
}