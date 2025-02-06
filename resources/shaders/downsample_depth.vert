#version 460 core

const vec3 VERTICES[] = vec3[](
    vec3(-1.f,  1.f, 0.f),
    vec3( 1.f,  1.f, 0.f),
    vec3(-1.f, -1.f, 0.f),
    vec3( 1.f, -1.f, 0.f)
);

void main() {
    gl_Position = vec4(VERTICES[gl_VertexID], 1.f);
}