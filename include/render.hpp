#ifndef RENDER_HPP
#define RENDER_HPP

#include <array>
#include <memory>

#include <GL/glew.h> 
#include <SDL2/SDL_video.h>

#include "buffer.hpp"
#include "framebuffer.hpp"
#include "light.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "texture.hpp"

enum struct DrawFlags : std::uint32_t {
    AmbientOcclusion = 1 << 0,
    Lighting = 1 << 1,
};

inline DrawFlags operator|(const DrawFlags a, const DrawFlags b) {
    return static_cast<DrawFlags>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

inline DrawFlags &operator|=(DrawFlags &a, const DrawFlags b) {
    return a = a | b;
}

inline DrawFlags operator&(const DrawFlags a, const DrawFlags b) {
    return static_cast<DrawFlags>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

inline DrawFlags &operator&=(DrawFlags &a, const DrawFlags b) {
    return a = a & b;
}

struct GpuCamera {
    glm::mat4   m_LastView;
    glm::mat4   m_Projection;
    glm::mat4   m_ProjectionInversed;
    glm::mat4   m_ProjectionNonReversed;
    glm::mat4   m_ProjectionNonReversedInversed;
    glm::mat4   m_View;
    glm::vec3   m_Position;
    float       m_Padding0;
    glm::vec2   m_NormTileDim;
    glm::vec2   m_TileSizeInv;
    float       m_FarZ;
    float       m_NearZ;
    float       m_FovX;
    float       m_FovY;
    float       m_SliceBiasFactor;
    float       m_SliceScalingFactor;
    float       m_Padding1;
    float       m_Padding2;
};

struct GpuCluster {
    glm::vec3   m_BoundsMax;
    float       m_Padding0;
    glm::mat4   m_BoundsMin;
    float       m_Padding1;
};

typedef std::uint32_t GpuIndex;

struct GpuLightEnvironment {
    std::array<glm::mat4, 5>    m_CascadeViewProjections;
    std::array<float, 4>        m_CascadePlaneDistances;
    glm::vec3                   m_AmbientColor;
    float                       m_Padding0;  
    glm::vec3                   m_BaseColor;
    float                       m_Padding1;  
    glm::vec3                   m_Direction;
    float                       m_Padding2;
};

struct GpuLightGrid {
    std::uint32_t   m_Count;
    std::uint32_t   m_Offset;
    float           m_Padding0;
    float           m_Padding1;
};

struct GpuLightPoint {
    std::array<glm::mat4, 6>    m_ViewProjections;
    glm::vec3                   m_Position;
    float                       m_Radius;  
    glm::vec3                   m_BaseColor;
    std::int32_t                m_ShadowIndex;
};

struct GpuMaterial {
    GLuint  m_DiffuseMap;
    GLuint  m_MetalnessMap;
    GLuint  m_NormalMap;
    GLuint  m_RoughnessMap;
};

typedef Vertex GpuVertex;

class Render {
public:
    Render();
    ~Render();

    void                                                    LoadModel(const Model &);
    void                                                    Update();

    float                                                   m_AmbientOcclusionFalloffFar;
    float                                                   m_AmbientOcclusionFalloffNear;
    std::int32_t                                            m_AmbientOcclusionNumSamples;
    std::int32_t                                            m_AmbientOcclusionNumSlices;
    float                                                   m_AmbientOcclusionRadius;
    SDL_GLContext                                           m_Context;
    DrawFlags                                               m_DrawFlags;
    const Camera *                                          m_DrawableActiveCamera;
    const LightEnvironment *                                m_DrawableLightEnvironment;
    std::vector<const LightPoint *>                         m_DrawableLightPoints;
    bool                                                    m_EnableAmbientOcclusion;
    bool                                                    m_EnableReverseZ;
    bool                                                    m_EnableVSync;
    bool                                                    m_EnableWireframeMode;
    float                                                   m_ShadowCsmFilterRadius;
    float                                                   m_ShadowCsmVarianceMax;
    float                                                   m_ShadowCubeFilterRadius;
    float                                                   m_ShadowCubeVarianceMax;

private:
    void                                                    ShadowCsmPass();
    void                                                    ShadowCubePass();
    void                                                    DepthPass();
    void                                                    DownsampleDepthPass();
    void                                                    AmbientOcclusionPass();
    void                                                    AmbientOcclusionSpartialPass();
    void                                                    AmbientOcclusionTemporalPass();
    void                                                    ClusterPass();
    void                                                    LightCullingPass();
    void                                                    LightingPass();
    void                                                    ScreenPass();
    
    std::unique_ptr<const Framebuffer>                      m_AmbientOcclusionFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_AmbientOcclusionShaderProgram;
    std::unique_ptr<const Texture2D>                        m_AmbientOcclusionTexture2D;
    std::unique_ptr<const Framebuffer>                      m_AmbientOcclusionSpartialFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_AmbientOcclusionSpartialShaderProgram;
    std::unique_ptr<const Texture2D>                        m_AmbientOcclusionSpartialTexture2D;
    std::unique_ptr<const Framebuffer>                      m_AmbientOcclusionTemporalFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_AmbientOcclusionTemporalShaderProgram;
    std::unique_ptr<const Texture2D>                        m_AmbientOcclusionTemporalTexture2D;
    std::unique_ptr<const Buffer<GpuCamera>>                m_CameraBuffer;
    std::unique_ptr<const Buffer<GpuCluster>>               m_ClusterBuffer;
    std::unique_ptr<const ShaderProgram>                    m_ClusterShaderProgram;
    std::unique_ptr<const Framebuffer>                      m_DepthFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_DepthShaderProgram;
    std::unique_ptr<const Texture2D>                        m_DepthTexture2D;
    std::vector<std::unique_ptr<const TextureView2D>>       m_DepthTextureView2Ds;
    std::unique_ptr<const Texture2DArray>                   m_DiffuseTexture2DArray;
    std::unique_ptr<const Framebuffer>                      m_DownsampleDepthFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_DownsampleDepthShaderProgram;
    std::unique_ptr<const DrawIndirectBuffer>               m_DrawIndirectBuffer;
    std::unique_ptr<const Buffer<GpuIndex>>                 m_IndexBuffer;
    std::unique_ptr<const Texture2D>                        m_LastAmbientOcclusionTemporalTexture2D;
    std::unique_ptr<const Framebuffer>                      m_LastDepthFramebuffer;
    std::unique_ptr<const Texture2D>                        m_LastDepthTexture2D;
    std::vector<std::unique_ptr<const TextureView2D>>       m_LastDepthTextureView2Ds;
    std::unique_ptr<const ShaderProgram>                    m_LastDownsampleDepthFramebuffer;
    std::unique_ptr<const Framebuffer>                      m_LastLightingFramebuffer;
    std::unique_ptr<const Buffer<std::uint32_t>>            m_LightCounterBuffer;
    std::unique_ptr<const ShaderProgram>                    m_LightCullingShaderProgram;
    std::unique_ptr<const Buffer<GpuLightEnvironment>>      m_LightEnvironmentBuffer;
    std::unique_ptr<const Buffer<GpuLightGrid>>             m_LightGridBuffer;
    std::unique_ptr<const Buffer<std::uint32_t>>            m_LightIndexBuffer;
    std::unique_ptr<const Buffer<GpuLightPoint>>            m_LightPointBuffer;
    std::unique_ptr<const Framebuffer>                      m_LightingFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_LightingShaderProgram;
    std::unique_ptr<const Texture2D>                        m_LightingTexture2D;
    std::unique_ptr<const Buffer<GpuMaterial>>              m_MaterialBuffer;
    std::vector<std::tuple<GLuint, GLuint>>                 m_Meshes;
    std::unique_ptr<const Texture2DArray>                   m_MetalnessTexture2DArray;
    std::unique_ptr<const Texture2DArray>                   m_NormalTexture2DArray;
    std::uint32_t                                           m_NumFrames;
    std::unique_ptr<const Texture2DArray>                   m_RoughnessTexture2DArray;
    std::unique_ptr<const Sampler>                          m_SamplerBorderWhite;
    std::unique_ptr<const Sampler>                          m_SamplerClamp;
    std::unique_ptr<const Sampler>                          m_SamplerWrap;
    std::unique_ptr<const ShaderProgram>                    m_ScreenShaderProgram;
    std::unique_ptr<const Texture2DArray>                   m_ShadowCsmColorTexture2DArray;
    std::unique_ptr<const Texture2DArray>                   m_ShadowCsmDepthTexture2DArray;
    std::unique_ptr<const Framebuffer>                      m_ShadowCsmFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_ShadowCsmShaderProgram;
    std::unique_ptr<const TextureCubeArray>                 m_ShadowCubeColorTextureCubeArray;
    std::vector<std::unique_ptr<const TextureViewCube>>     m_ShadowCubeColorTextureViewCubes;
    std::unique_ptr<const TextureCubeArray>                 m_ShadowCubeDepthTextureCubeArray;
    std::vector<std::unique_ptr<const TextureViewCube>>     m_ShadowCubeDepthTextureViewCubes;
    std::unique_ptr<const Framebuffer>                      m_ShadowCubeFramebuffer;
    std::unique_ptr<const ShaderProgram>                    m_ShadowCubeShaderProgram;
    std::unique_ptr<const Buffer<GpuVertex>>                m_VertexBuffer;
};

extern std::unique_ptr<Render> g_Render;

#endif /* RENDER_HPP */