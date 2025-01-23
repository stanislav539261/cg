#ifndef RENDER_HPP
#define RENDER_HPP

#include <memory>

#include <GL/glew.h> 
#include <SDL2/SDL_video.h>

#include "buffer.hpp"
#include "framebuffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include "scene.hpp"

struct GpuCamera {
    glm::mat4 m_Projection;
    glm::mat4 m_View;
};

typedef unsigned int GpuIndex;

struct GpuMaterial {
    GLuint m_DiffuseMap;
    GLuint m_MetalnessMap;
    GLuint m_NormalMap;
    GLuint m_RoughnessMap;
};

typedef Vertex GpuVertex;

class Render {
public:
    Render(SDL_Window *);
    ~Render();

    void                                    LoadScene(const Scene &);
    void                                    Update();

    SDL_GLContext                         m_Context;

private:
    void                                    LightingPass();
    void                                    ScreenPass();
    
    std::shared_ptr<Buffer<GpuCamera>>      m_CameraBuffer;
    std::shared_ptr<Texture2D>              m_DepthTexture2D;
    std::shared_ptr<Texture2DArray>         m_DiffuseTexture2DArray;
    std::shared_ptr<DrawIndirectBuffer>     m_DrawIndirectBuffer;
    bool                                    m_EnableReverseZ;
    std::shared_ptr<Buffer<GpuIndex>>       m_IndexBuffer;
    std::shared_ptr<Framebuffer>            m_LightingFramebuffer;
    std::shared_ptr<ShaderProgram>          m_LightingShaderProgram;
    std::shared_ptr<Texture2D>              m_LightingTexture2D;
    std::shared_ptr<Buffer<GpuMaterial>>    m_MaterialBuffer;
    std::vector<std::tuple<GLuint, GLuint>> m_Meshes;
    std::shared_ptr<Texture2DArray>         m_MetalnessTexture2DArray;
    std::shared_ptr<Texture2DArray>         m_NormalTexture2DArray;
    std::shared_ptr<Texture2DArray>         m_RoughnessTexture2DArray;
    std::shared_ptr<Sampler>                m_SamplerWrap;
    std::shared_ptr<DefaultFramebuffer>     m_ScreenFramebuffer;
    std::shared_ptr<ShaderProgram>          m_ScreenShaderProgram;
    std::shared_ptr<Buffer<GpuVertex>>      m_VertexBuffer;
};

extern std::shared_ptr<Render> g_Render;

#endif /* RENDER_HPP */