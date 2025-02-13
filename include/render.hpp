#ifndef RENDER_HPP
#define RENDER_HPP

#include <array>
#include <memory>

#include <GL/glew.h> 
#include <SDL2/SDL_video.h>

#include "buffer.hpp"
#include "framebuffer.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "texture.hpp"

enum struct DrawFlags : unsigned int {
    AmbientOcclusion = 1 << 0,
    Lighting = 1 << 1,
};

inline DrawFlags operator|(const DrawFlags a, const DrawFlags b) {
    return static_cast<DrawFlags>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

inline DrawFlags &operator|=(DrawFlags &a, const DrawFlags b) {
    return a = a | b;
}

inline DrawFlags operator&(const DrawFlags a, const DrawFlags b) {
    return static_cast<DrawFlags>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
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

typedef unsigned int GpuIndex;

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
    unsigned int    m_Count;
    unsigned int    m_Offset;
    float           m_Padding0;
    float           m_Padding1;
};

struct GpuLightPoint {
    std::array<glm::mat4, 6>    m_ViewProjections;
    glm::vec3                   m_Position;
    float                       m_Radius;  
    glm::vec3                   m_BaseColor;
    int                         m_ShadowIndex;
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

    void                                            LoadModel(const Model &);
    void                                            Update();

    float                                           m_AmbientOcclusionFalloffFar;
    float                                           m_AmbientOcclusionFalloffNear;
    int                                             m_AmbientOcclusionNumSamples;
    int                                             m_AmbientOcclusionNumSlices;
    float                                           m_AmbientOcclusionRadius;
    SDL_GLContext                                   m_Context;
    DrawFlags                                       m_DrawFlags;
    bool                                            m_EnableAmbientOcclusion;
    bool                                            m_EnableReverseZ;
    bool                                            m_EnableVSync;
    bool                                            m_EnableWireframeMode;
    float                                           m_ShadowCsmFilterRadius;
    float                                           m_ShadowCsmVarianceMax;
    float                                           m_ShadowCubeFilterRadius;
    float                                           m_ShadowCubeVarianceMax;

private:
    void                                            ShadowCsmPass();
    void                                            ShadowCubePass();
    void                                            DepthPass();
    void                                            DownsampleDepthPass();
    void                                            AmbientOcclusionPass();
    void                                            AmbientOcclusionSpartialPass();
    void                                            AmbientOcclusionTemporalPass();
    void                                            ClusterPass();
    void                                            LightCullingPass();
    void                                            LightingPass();
    void                                            ScreenPass();
    
    std::shared_ptr<Framebuffer>                    m_AmbientOcclusionFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_AmbientOcclusionShaderProgram;
    std::shared_ptr<Texture2D>                      m_AmbientOcclusionTexture2D;
    std::shared_ptr<Framebuffer>                    m_AmbientOcclusionSpartialFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_AmbientOcclusionSpartialShaderProgram;
    std::shared_ptr<Texture2D>                      m_AmbientOcclusionSpartialTexture2D;
    std::shared_ptr<Framebuffer>                    m_AmbientOcclusionTemporalFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_AmbientOcclusionTemporalShaderProgram;
    std::shared_ptr<Texture2D>                      m_AmbientOcclusionTemporalTexture2D;
    std::shared_ptr<Buffer<GpuCamera>>              m_CameraBuffer;
    std::shared_ptr<Buffer<GpuCluster>>             m_ClusterBuffer;
    std::shared_ptr<ShaderProgram>                  m_ClusterShaderProgram;
    std::shared_ptr<Framebuffer>                    m_DepthFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_DepthShaderProgram;
    std::shared_ptr<Texture2D>                      m_DepthTexture2D;
    std::vector<std::shared_ptr<TextureView2D>>     m_DepthTextureView2Ds;
    std::shared_ptr<Texture2DArray>                 m_DiffuseTexture2DArray;
    std::shared_ptr<Framebuffer>                    m_DownsampleDepthFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_DownsampleDepthShaderProgram;
    std::shared_ptr<DrawIndirectBuffer>             m_DrawIndirectBuffer;
    std::shared_ptr<Buffer<GpuIndex>>               m_IndexBuffer;
    std::shared_ptr<Texture2D>                      m_LastAmbientOcclusionTemporalTexture2D;
    std::shared_ptr<Framebuffer>                    m_LastDepthFramebuffer;
    std::shared_ptr<Texture2D>                      m_LastDepthTexture2D;
    std::vector<std::shared_ptr<TextureView2D>>     m_LastDepthTextureView2Ds;
    std::shared_ptr<ShaderProgram>                  m_LastDownsampleDepthFramebuffer;
    std::shared_ptr<Framebuffer>                    m_LastLightingFramebuffer;
    std::shared_ptr<Buffer<unsigned int>>           m_LightCounterBuffer;
    std::shared_ptr<ShaderProgram>                  m_LightCullingShaderProgram;
    std::shared_ptr<Buffer<GpuLightEnvironment>>    m_LightEnvironmentBuffer;
    std::shared_ptr<Buffer<GpuLightGrid>>           m_LightGridBuffer;
    std::shared_ptr<Buffer<unsigned int>>           m_LightIndexBuffer;
    std::shared_ptr<Buffer<GpuLightPoint>>          m_LightPointBuffer;
    std::shared_ptr<Framebuffer>                    m_LightingFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_LightingShaderProgram;
    std::shared_ptr<Texture2D>                      m_LightingTexture2D;
    std::shared_ptr<Buffer<GpuMaterial>>            m_MaterialBuffer;
    std::vector<std::tuple<GLuint, GLuint>>         m_Meshes;
    std::shared_ptr<Texture2DArray>                 m_MetalnessTexture2DArray;
    std::shared_ptr<Texture2DArray>                 m_NormalTexture2DArray;
    unsigned int                                    m_NumFrames;
    std::shared_ptr<Texture2DArray>                 m_RoughnessTexture2DArray;
    std::shared_ptr<Sampler>                        m_SamplerBorderWhite;
    std::shared_ptr<Sampler>                        m_SamplerClamp;
    std::shared_ptr<Sampler>                        m_SamplerWrap;
    std::shared_ptr<DefaultFramebuffer>             m_ScreenFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_ScreenShaderProgram;
    std::shared_ptr<Texture2DArray>                 m_ShadowCsmColorTexture2DArray;
    std::shared_ptr<Texture2DArray>                 m_ShadowCsmDepthTexture2DArray;
    std::shared_ptr<Framebuffer>                    m_ShadowCsmFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_ShadowCsmShaderProgram;
    std::shared_ptr<TextureCubeArray>               m_ShadowCubeColorTextureCubeArray;
    std::vector<std::shared_ptr<TextureViewCube>>   m_ShadowCubeColorTextureViewCubes;
    std::shared_ptr<TextureCubeArray>               m_ShadowCubeDepthTextureCubeArray;
    std::vector<std::shared_ptr<TextureViewCube>>   m_ShadowCubeDepthTextureViewCubes;
    std::shared_ptr<Framebuffer>                    m_ShadowCubeFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_ShadowCubeShaderProgram;
    std::shared_ptr<Buffer<GpuVertex>>              m_VertexBuffer;
};

extern std::shared_ptr<Render> g_Render;

#endif /* RENDER_HPP */