#version 460 core

struct LightEnvironment {
    mat4  m_CascadeViewProjections[5];
    vec4  m_CascadePlaneDistances;
    vec3  m_AmbientColor;
    float m_Padding0;
    vec3  m_BaseColor;
    float m_Padding1;
    vec3  m_Direction;
    float m_Padding2;
};

struct Material {
    uint m_DiffuseMap;
    uint m_MetalnessMap;
    uint m_NormalMap;
    uint m_RoughnessMap;
};

layout(std430, binding = 0) readonly buffer CameraBuffer {
    mat4  g_Projection;
    mat4  g_View;
    float g_FarZ;
    float g_NearZ;
    float m_Padding0;
    float m_Padding1;
};

layout(std430, binding = 2) readonly buffer LightEnvironmentBuffer {
    LightEnvironment g_LightEnvironment;
};

layout(std430, binding = 3) readonly buffer MaterialBuffer {
    Material g_Materials[];
};

layout(binding = 0) uniform sampler2DArray g_DiffuseTextures;
layout(binding = 1) uniform sampler2DArray g_MetalnessTextures;
layout(binding = 2) uniform sampler2DArray g_NormalTextures;
layout(binding = 3) uniform sampler2DArray g_RoughnessTextures;
layout(binding = 4) uniform sampler2DArray g_ShadowCsmColorTextures;
layout(binding = 5) uniform sampler2DArray g_ShadowCsmDepthTextures;

layout(location = 0) uniform bool g_EnableReverseZ;
layout(location = 1) uniform float g_ShadowSizeInv;

in VS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) smooth vec2 m_Texcoord;
    layout(location = 2) smooth vec3 m_Normal;
    layout(location = 3) flat uint m_Material;
} VS_Output;

layout(location = 0) out vec4 outColor;

const uint NUM_CASCADES = 4;
const mat4 PROJECTOR_BIAS = mat4(0.5f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.5f, 0.5f, 0.f, 1.f);

const float SHADOW_AMOUNT = 0.3f;
const float SHADOW_BIAS = 0.000008f;
const vec2 SHADOW_POISSON[] = vec2[16](
    vec2( -0.94201624f, -0.39906216f ),
    vec2( 0.94558609f, -0.76890725f ),
    vec2( -0.094184101f, -0.92938870f ),
    vec2( 0.34495938f, 0.29387760f ),
    vec2( -0.91588581f, 0.45771432f ),
    vec2( -0.81544232f, -0.87912464f ),
    vec2( -0.38277543f, 0.27676845f ),
    vec2( 0.97484398f, 0.75648379f ),
    vec2( 0.44323325f, -0.97511554f ),
    vec2( 0.53742981f, -0.47373420f ),
    vec2( -0.26496911f, -0.41893023f ),
    vec2( 0.79197514f, 0.19090188f ),
    vec2( -0.24188840f, 0.99706507f ),
    vec2( -0.81409955f, 0.91437590f ),
    vec2( 0.19984126f, 0.78641367f ),
    vec2( 0.14383161f, -0.14100790f )
);

float ComputeShadowCsm(const vec3 fragPos, const vec3 normal, const vec3 lightDir) {
    vec4 fragPosViewSpace = g_View * vec4(fragPos, 1.f);
    float depth = abs(fragPosViewSpace.z);

    uint layer = NUM_CASCADES;

    for (uint i = 0; i < NUM_CASCADES; ++i) {
        if (depth < g_LightEnvironment.m_CascadePlaneDistances[i]) {
            layer = i;
            break;
        }
    }

    vec4 projCoords = PROJECTOR_BIAS * g_LightEnvironment.m_CascadeViewProjections[layer] * vec4(fragPos, 1.f);
    vec3 projCoordsW = projCoords.xyz / projCoords.w;
    float shadow = 0.f;

    if (projCoords.w > 0.f && (max(projCoordsW.x, projCoordsW.y) > 0.f) && (min(projCoordsW.x, projCoordsW.y) < 1.f)) {
        // const float biasModifier = 0.5f;

        // float bias = max(0.05f * (1.f - dot(normal, lightDir)), 0.005f);

        // if (layer == NUM_CASCADES) {
        //     bias *= 1.f / (g_FarZ * biasModifier);
        // } else {
        //     bias *= 1.f / (g_LightEnvironment.m_CascadePlaneDistances[layer] * biasModifier);
        // }

        const float filterRadius = g_ShadowSizeInv * 2.f;

        for (uint i = 0; i < 16; i++) {
            vec2 poisson = SHADOW_POISSON[i];
            vec2 offset = poisson * filterRadius;
            float momentX = textureLod(g_ShadowCsmDepthTextures, vec3(projCoords.xy + offset, layer), 0).r;
            float momentY = textureLod(g_ShadowCsmColorTextures, vec3(projCoords.xy + offset, layer), 0).r;

            if (g_EnableReverseZ) {
                if (projCoords.z > momentX) {
                    shadow += 1.f;
                } else {
                    float variance = min(momentY - (momentX * momentX), 1.f - 0.000002f);
                    float p = step(projCoords.z, momentX);
                    float distX = projCoords.z - momentX;
                    float pMin = variance / (variance + distX * distX);

                    shadow += clamp((min(p, pMin) - SHADOW_AMOUNT) / (1.f - SHADOW_AMOUNT), 0.f, 1.f);
                }
            } else {
                if (projCoords.z < momentX) {
                    shadow += 1.f;
                } else {
                    float variance = max(momentY - (momentX * momentX), 0.000002f);
                    float p = step(projCoords.z, momentX);
                    float distX = projCoords.z - momentX;
                    float pMax = variance / (variance + distX * distX);

                    shadow += clamp((max(p, pMax) - SHADOW_AMOUNT) / (1.f - SHADOW_AMOUNT), 0.f, 1.f);
                }
            }
        }

        shadow *= 1.f / 16.f;
    } else {
        shadow = 1.f;
    }

    return shadow;
}

void main() {
    uint diffuseMap = g_Materials[VS_Output.m_Material].m_DiffuseMap;
    vec4 diffuseColor = texture(g_DiffuseTextures, vec3(VS_Output.m_Texcoord, diffuseMap));

    vec3 fragPos = VS_Output.m_FragPos;
    vec3 normal = normalize(VS_Output.m_Normal);
    vec3 lightDir = normalize(-g_LightEnvironment.m_Direction);  
    float diff = max(dot(normal, lightDir), 0.f);

    vec3 ambientLighting = g_LightEnvironment.m_AmbientColor * diffuseColor.rgb;
    vec3 diffuseLighting = g_LightEnvironment.m_BaseColor * diffuseColor.rgb * diff;  

    outColor = vec4(ambientLighting + diffuseLighting * ComputeShadowCsm(fragPos, normal, lightDir), 1.f);
}