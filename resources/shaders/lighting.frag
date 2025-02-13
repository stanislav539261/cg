#version 460 core

#define GRID_SIZE_X 16
#define GRID_SIZE_Y 8
#define GRID_SIZE_Z 24

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

struct LightGrid {
    uint  m_Count;
    uint  m_Offset;
    float m_Padding0;
    float m_Padding1;
};

struct LightPoint {
    mat4  m_ViewProjections[6];
    vec3  m_Position;
    float m_Radius;
    vec3  m_BaseColor;
    int   m_ShadowIndex;
};

struct Material {
    uint m_DiffuseMap;
    uint m_MetalnessMap;
    uint m_NormalMap;
    uint m_RoughnessMap;
};

layout(std430, binding = 0) readonly buffer CameraBuffer {
    mat4  g_LastView;
    mat4  g_Projection;
    mat4  g_ProjectionInversed;
    mat4  g_ProjectionNonReversed;
    mat4  g_ProjectionNonReversedInversed;
    mat4  g_View;
    vec3  g_CameraPos;
    float m_Padding0;
    vec2  g_NormTileDim;
    vec2  g_TileSizeInv;
    float g_FarZ;
    float g_NearZ;
    float g_FovX;
    float g_FovY;
    float g_SliceBiasFactor;
    float g_SliceScalingFactor;
    float m_Padding1;
    float m_Padding2;
};

layout(std430, binding = 2) readonly buffer LightEnvironmentBuffer {
    LightEnvironment g_LightEnvironment;
};

layout(std430, binding = 3) readonly buffer LightGridBuffer {
    LightGrid g_LightGrids[];
};

layout(std430, binding = 4) readonly buffer LightIndexBuffer {
    uint g_LightIndices[];
};

layout(std430, binding = 5) readonly buffer LightPointBuffer {
    LightPoint g_LightPoints[];
};

layout(std430, binding = 6) readonly buffer MaterialBuffer {
    Material g_Materials[];
};

layout(binding = 0) uniform sampler2D g_AmbientOcclusionTexture;
layout(binding = 1) uniform sampler2DArray g_DiffuseTextures;
layout(binding = 2) uniform sampler2DArray g_MetalnessTextures;
layout(binding = 3) uniform sampler2DArray g_NormalTextures;
layout(binding = 4) uniform sampler2DArray g_RoughnessTextures;
layout(binding = 5) uniform sampler2DArray g_ShadowCsmColorTextures;
layout(binding = 6) uniform sampler2DArray g_ShadowCsmDepthTextures;
layout(binding = 7) uniform samplerCubeArray g_ShadowCubeColorTextures;
layout(binding = 8) uniform samplerCubeArray g_ShadowCubeDepthTextures;

layout(location = 0) uniform bool g_EnableAmbientOcclusion;
layout(location = 1) uniform bool g_EnableReverseZ;
layout(location = 2) uniform uint g_NumLightPoints;
layout(location = 3) uniform float g_ShadowCsmFilterRadius;
layout(location = 4) uniform float g_ShadowCsmVarianceMax;
layout(location = 5) uniform float g_ShadowCubeFilterRadius;
layout(location = 6) uniform float g_ShadowCubeVarianceMax;

in VS_OUT {
    layout(location = 0) smooth vec3 m_FragPos;
    layout(location = 1) smooth vec2 m_Texcoord;
    layout(location = 2) smooth vec3 m_Normal;
    layout(location = 3) flat uint m_Material;
} VS_Output;

layout(location = 0) out vec4 outColor;

const float M_PI = 3.1415926535897932384626433832795f;
const uint NUM_CASCADES = 4;
const mat4 PROJECTOR_BIAS = mat4(0.5f, 0.f, 0.f, 0.f, 0.f, 0.5f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.5f, 0.5f, 0.f, 1.f);
const float SHADOW_AMOUNT = 0.3f;
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

        for (uint i = 0; i < 16; i++) {
            vec2 poisson = SHADOW_POISSON[i];
            vec2 offset = poisson * g_ShadowCsmFilterRadius;
            float momentX = textureLod(g_ShadowCsmDepthTextures, vec3(projCoords.xy + offset, layer), 0).r;
            float momentY = textureLod(g_ShadowCsmColorTextures, vec3(projCoords.xy + offset, layer), 0).r;

            if (g_EnableReverseZ) {
                if (projCoords.z > momentX) {
                    shadow += 1.f;
                } else {
                    float variance = min(momentY - (momentX * momentX), 1.f - g_ShadowCsmVarianceMax);
                    float p = step(projCoords.z, momentX);
                    float distX = projCoords.z - momentX;
                    float pMin = variance / (variance + distX * distX);

                    shadow += clamp((min(p, pMin) - SHADOW_AMOUNT) / (1.f - SHADOW_AMOUNT), 0.f, 1.f);
                }
            } else {
                if (projCoords.z < momentX) {
                    shadow += 1.f;
                } else {
                    float variance = max(momentY - (momentX * momentX), g_ShadowCsmVarianceMax);
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

float ComputeShadowCube(const vec3 lightDir, const float lightZ, const uint layer) {
    vec3 up;

    if ((abs(lightDir.x) < 0.0001f) && (abs(lightDir.z) < 0.0001f)) {
        up = (lightDir.y > 0.f) ? vec3(0.f, 0.f, -1.f) : vec3(0.f, 0.f, 1.f);
    } else {
        up = vec3(0.f, 1.f, 0.f);
    }

    const vec3 right = normalize(cross(up, lightDir));

    up = normalize(cross(lightDir, right));

    const mat3 rotation = mat3(right, up, -lightDir);

    float shadow = 0.f;

    for (uint i = 0; i < 16; i++) {
        const vec2 poisson = SHADOW_POISSON[i];

        vec4 texcoord;

        texcoord.xy = poisson * g_ShadowCubeFilterRadius;
        texcoord.z = -sqrt(1.f - dot(texcoord.xy, texcoord.xy));
        texcoord.w = float(layer);
        texcoord.xyz = rotation * texcoord.xyz;

        const float momentX = texture(g_ShadowCubeDepthTextures, texcoord).r;
        const float momentY = texture(g_ShadowCubeColorTextures, texcoord).r;
        const float variance = max(momentY - (momentX * momentX), g_ShadowCubeVarianceMax);
        const float penumbra = step(lightZ, momentX);
        const float distX = lightZ - momentX;
        const float penumbraMax = variance / (variance + distX * distX);

        shadow += clamp((max(penumbra, penumbraMax) - SHADOW_AMOUNT) / (1.f - SHADOW_AMOUNT), 0.f, 1.f);
    }
    
    return shadow * 1.f / 16.f;
}

vec3 ComputeFresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (vec3(1.f) - F0) * pow(1.f - cosTheta, 5.f);
}

float ComputeGeometrySchlickGGX(float cosLi, float cosLo, float roughness) {
    float r = roughness + 1.f;
    float k = (r * r) / 8.f;
    float termLi = cosLi * 1.f / (cosLi * (1.f - k) + k);
    float termLo = cosLo * 1.f / (cosLo * (1.f - k) + k);
    
    return termLi * termLo;
}

float ComputeNdfGGX(float cosLh, float roughness) {
    float roughness2 = roughness * roughness;
    float roughness4 = roughness2 * roughness2;
    float denom = (cosLh * cosLh) * (roughness4 - 1.0) + 1.0;
    return roughness4 / (M_PI * denom * denom);
}

vec3 ComputePBR(vec3 normal, vec3 lightDir, vec3 viewDir, vec3 F0, vec3 albedo, float metalness, float roughness) {
    vec3 halfwayDir = normalize(viewDir + lightDir);

    float cosLo = max(0.f, dot(normal, viewDir));
    float cosLi = max(0.f, dot(normal, lightDir));
    float cosLh = max(0.f, dot(normal, halfwayDir));

    // Cook-Torrance BRDF
    vec3 F = ComputeFresnelSchlick(F0, max(dot(halfwayDir, viewDir), 0.f));        
    float NDF = ComputeNdfGGX(cosLi, roughness);   
    float G = ComputeGeometrySchlickGGX(cosLi, cosLo, roughness);    
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.f * max(dot(normal, viewDir), 0.f) * max(dot(normal, lightDir), 0.f) + 0.0001f;
    vec3 specular = numerator / denominator;
    
    vec3 kD = mix(vec3(1.f) - F, vec3(0.f), metalness);      

    const float EPSILON = 0.00001f;  
        
    vec3 diffuseBRDF = kD * albedo;
    vec3 specularBRDF = (F * NDF * G) / max(EPSILON, 4.f * cosLi * cosLo);

    return (diffuseBRDF + specularBRDF) * cosLi;
}

mat3 ComputeTBN(vec3 normal, vec3 p, vec2 uv) {
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
 
    vec3 dp2perp = cross(dp2, normal);
    vec3 dp1perp = cross(normal, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));

    return mat3(T * invmax, B * invmax, normal);
}

vec3 ComputeGtaoMultiBounce(float ao, vec3 albedo) {
    vec3 x = vec3(ao);
    vec3 a = 2.0404 * albedo - vec3(0.3324);
    vec3 b = -4.7951 * albedo + vec3(0.6417);
    vec3 c = 2.7552 * albedo + vec3(0.6903);

    return max(x, ((x * a + b) * x + c) * x);
}

float LinearizeZ(const float depth, const float near, const float far) {
    return near * far / (depth * (near - far) + far);
}

vec3 ComputeLighting(
    const vec3 fragPos, 
    const vec2 texcoord, 
    const vec3 normal, 
    const vec3 viewDir, 
    const vec3 albedo,
    const float metalness,
    const float roughness
) {
    vec3 lighting = vec3(0.f);

    const vec3 F0 = mix(vec3(0.04f), albedo, metalness);

    // Add direct environment light
    const vec3 globalLightDir = -g_LightEnvironment.m_Direction;
    const vec3 globalLighting = g_LightEnvironment.m_BaseColor * ComputePBR(normal, globalLightDir, viewDir, F0, albedo, metalness, roughness);  
    lighting += globalLighting * ComputeShadowCsm(fragPos, normal, globalLightDir);

    // Add local lights
    const float z = g_EnableReverseZ ? LinearizeZ(gl_FragCoord.z, g_FarZ, g_NearZ) : LinearizeZ(gl_FragCoord.z, g_NearZ, g_FarZ);
    const uint slice = uint(log2(z) * g_SliceScalingFactor + g_SliceBiasFactor);
    const uvec3 tile3 = uvec3(uvec2(gl_FragCoord.xy * g_TileSizeInv), slice);
    const uint tile = tile3.x + GRID_SIZE_X * tile3.y + GRID_SIZE_X * GRID_SIZE_Y * tile3.z;
    const uint offset = g_LightGrids[tile].m_Offset;

    for (uint i = 0; i < g_LightGrids[tile].m_Count; i++) {
        const uint lightIndex = g_LightIndices[offset + i];
        const vec3 lightPos = g_LightPoints[lightIndex].m_Position - fragPos;
        const float lightLength = length(lightPos);
        const float lightDist = lightLength / g_LightPoints[lightIndex].m_Radius;

        if (lightDist < 1.f) {
            const vec3 lightDir = lightPos * 1.f / lightLength;
            const float lightDist2 = lightDist * lightDist;
            const float lightDist2Rev = 1.f - lightDist2;
            const float lightDist2Rev2 = lightDist2Rev * lightDist2Rev;
            const float lightAttenuation = lightDist2Rev2 * 1.f / (1.f + lightDist);

            vec3 localLighting = g_LightPoints[lightIndex].m_BaseColor * ComputePBR(normal, lightDir, viewDir, F0, albedo, metalness, roughness);  
            localLighting *= lightAttenuation;

            const int shadowIndex = g_LightPoints[lightIndex].m_ShadowIndex;

            if (shadowIndex != -1) {
                localLighting *= ComputeShadowCube(-lightDir, lightDist, uint(shadowIndex));
            }

            lighting += localLighting;
        }
    }

    // Add ambient environment light
    lighting += g_LightEnvironment.m_AmbientColor * albedo;
    
    const vec3 aoFactor = g_EnableAmbientOcclusion ? ComputeGtaoMultiBounce(texelFetch(g_AmbientOcclusionTexture, ivec2(gl_FragCoord.xy), 0).r, lighting) : vec3(1.f);

    return lighting * aoFactor;
}

void main() {
    const uint material = VS_Output.m_Material;
    const vec2 texcoord = VS_Output.m_Texcoord;

    const uint diffuseMap = g_Materials[material].m_DiffuseMap;
    const vec4 diffuseColor = texture(g_DiffuseTextures, vec3(texcoord, diffuseMap));
    const uint metalnessMap = g_Materials[material].m_MetalnessMap;
    const vec4 metalnessColor = texture(g_MetalnessTextures, vec3(texcoord, metalnessMap));
    const uint normalMap = g_Materials[material].m_NormalMap;
    const vec4 normalColor = texture(g_NormalTextures, vec3(texcoord, normalMap));
    const uint roughnessMap = g_Materials[material].m_RoughnessMap;
    const vec4 roughnessColor = texture(g_RoughnessTextures, vec3(texcoord, roughnessMap));

    const vec3 fragPos = VS_Output.m_FragPos;
    const vec3 viewPos = g_CameraPos - fragPos;
    const vec3 normal = normalize(ComputeTBN(VS_Output.m_Normal, -viewPos, texcoord) * (normalColor.rgb * 2.f - 1.f));
    const vec3 viewDir = normalize(viewPos);

    vec3 lighting = vec3(0.f);

    outColor = vec4(ComputeLighting(fragPos, texcoord, normal, viewDir, diffuseColor.rgb, metalnessColor.r, roughnessColor.r), 1.f);
}