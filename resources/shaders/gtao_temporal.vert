#version 460 core

out VS_OUT {
    layout(location = 0) smooth vec2 m_Texcoord;
} VS_Output;

const vec3 VERTICES[] = vec3[](
    vec3(-1.f,  1.f, 0.f),
    vec3( 1.f,  1.f, 0.f),
    vec3(-1.f, -1.f, 0.f),
    vec3( 1.f, -1.f, 0.f)
);

void main() {
    VS_Output.m_Texcoord = VERTICES[gl_VertexID].xy * 0.5f + 0.5f;

    gl_Position = vec4(VERTICES[gl_VertexID], 1.f);
}