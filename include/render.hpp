#ifndef RENDER_HPP
#define RENDER_HPP

#include <array>
#include <memory>

#include <GL/glew.h> 
#include <SDL2/SDL_video.h>

#include "buffer.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "scene.hpp"

struct GpuCamera {
    glm::mat4   m_Projection;
    glm::mat4   m_ProjectionInversed;
    glm::mat4   m_View;
    glm::vec3   m_Position;
    float       m_Padding0;
    float       m_FarZ;
    float       m_NearZ;
    float       m_FovX;
    float       m_FovY;
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

    void                                            LoadScene(const Scene &);
    void                                            Update();

    SDL_GLContext                                   m_Context;
    bool                                            m_EnableAmbientOcclusion;
    bool                                            m_EnableReverseZ;

private:
    void                                            ShadowCsmPass();
    void                                            DepthPass();
    void                                            VelocityPass();
    void                                            DownsampleDepthVelocityPass();
    void                                            AmbientOcclusionPass();
    void                                            AmbientOcclusionSpartialPass();
    void                                            AmbientOcclusionTemporalPass();
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
    std::shared_ptr<Framebuffer>                    m_DepthFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_DepthShaderProgram;
    std::shared_ptr<Texture2D>                      m_DepthTexture2D;
    std::shared_ptr<Texture2DArray>                 m_DiffuseTexture2DArray;
    std::shared_ptr<Framebuffer>                    m_DownsampleDepthVelocityFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_DownsampleDepthVelocityShaderProgram;
    std::shared_ptr<DrawIndirectBuffer>             m_DrawIndirectBuffer;
    std::shared_ptr<Texture2D>                      m_HalfDepthTexture2D;
    std::shared_ptr<Texture2D>                      m_HalfVelocityTexture2D;
    std::shared_ptr<Buffer<GpuIndex>>               m_IndexBuffer;
    std::shared_ptr<Framebuffer>                    m_LastAmbientOcclusionTemporalFramebuffer;
    std::shared_ptr<Texture2D>                      m_LastAmbientOcclusionTemporalTexture2D;
    std::shared_ptr<Framebuffer>                    m_LastDepthFramebuffer;
    std::shared_ptr<Texture2D>                      m_LastDepthTexture2D;
    std::shared_ptr<Framebuffer>                    m_LastDownsampleDepthVelocityFramebuffer;
    std::shared_ptr<Texture2D>                      m_LastHalfDepthTexture2D;
    std::shared_ptr<Framebuffer>                    m_LastLightingFramebuffer;
    std::shared_ptr<Buffer<GpuLightEnvironment>>    m_LightEnvironmentBuffer;
    std::shared_ptr<Framebuffer>                    m_LightingFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_LightingShaderProgram;
    std::shared_ptr<Texture2D>                      m_LightingTexture2D;
    std::shared_ptr<Buffer<GpuMaterial>>            m_MaterialBuffer;
    std::vector<std::tuple<GLuint, GLuint>>         m_Meshes;
    std::shared_ptr<Texture2DArray>                 m_MetalnessTexture2DArray;
    std::shared_ptr<Texture2DArray>                 m_NormalTexture2DArray;
    unsigned int                                    m_NumFrames;
    std::shared_ptr<Texture2DArray>                 m_RoughnessTexture2DArray;
    std::shared_ptr<Sampler>                        m_SamplerClamp;
    std::shared_ptr<Sampler>                        m_SamplerShadowBorder;
    std::shared_ptr<Sampler>                        m_SamplerWrap;
    std::shared_ptr<DefaultFramebuffer>             m_ScreenFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_ScreenShaderProgram;
    std::shared_ptr<Texture2DArray>                 m_ShadowCsmColorTexture2DArray;
    std::shared_ptr<Texture2DArray>                 m_ShadowCsmDepthTexture2DArray;
    std::shared_ptr<Framebuffer>                    m_ShadowCsmFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_ShadowCsmShaderProgram;
    std::shared_ptr<Framebuffer>                    m_VelocityFramebuffer;
    std::shared_ptr<ShaderProgram>                  m_VelocityShaderProgram;
    std::shared_ptr<Texture2D>                      m_VelocityTexture2D;
    std::shared_ptr<Buffer<GpuVertex>>              m_VertexBuffer;
};

extern std::shared_ptr<Render> g_Render;

#endif /* RENDER_HPP */