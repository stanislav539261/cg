#include <array>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>

#include "camera.hpp"
#include "light.hpp"
#include "render.hpp"
#include "state.hpp"
#include "window.hpp"

constexpr GLfloat COLOR_ONE[] = { 1.f, 1.f, 1.f, 1.f };
constexpr GLfloat COLOR_ZERO[] = { 0.f, 0.f, 0.f, 0.f };
constexpr GLfloat DEPTH_ONE[] = { 1.f };
constexpr GLfloat DEPTH_ZERO[] = { 0.f };
constexpr GLuint SHADOW_SIZE = 2048;

std::shared_ptr<Render> g_Render = nullptr;

static void GLAPIENTRY DebugMessageCallback(
    GLenum source, 
    GLenum type, 
    GLuint id, 
    GLenum severity, 
    GLsizei length,
    const GLchar *message,
    const GLvoid *data
) {
    std::cout << "error id: " << std::hex << id << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "source: API" << std::endl;
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "source: WINDOW SYSTEM" << std::endl;
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "source: SHADER COMPILER" << std::endl;
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "source: THIRD PARTY" << std::endl;
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "source: APPLICATION" << std::endl;
            break;
        default:
            std::cout << "source: UNKNOWN" << std::endl;
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "type: ERROR" << std::endl;
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "type: DEPRECATED BEHAVIOR" << std::endl;
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "type: UNDERFINED BEHAVIOR" << std::endl;
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "type: PORTABILITY" << std::endl;
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "type: PERFORMANCE" << std::endl;
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "type: OTHER" << std::endl;
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "type: MARKER" << std::endl;
            break;
        default:
            std::cout << "type: UNKNOWN" << std::endl;
            break;
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "severity: HIGH" << std::endl;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "severity: MEDIUM" << std::endl;
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "severity: LOW" << std::endl;
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "severity: NOTIFICATION" << std::endl;
            break;
        default:
            std::cout << "severity: UNKNOWN" << std::endl;
            break;
    }
    
    std::cout << "message: " << message << std::endl;

    throw std::runtime_error("error");
}

Render::Render() {
    if (g_Window) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        m_Context = SDL_GL_CreateContext(g_Window->m_Window);
    }

    if (m_Context) {
        auto result = glewInit();

        if (result == GLEW_OK) {
            glEnable(GL_DEPTH_CLAMP);
            glEnable(GL_DEBUG_OUTPUT);;
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glEnable(GL_SCISSOR_TEST);
            glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
            glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
            glDebugMessageCallback(DebugMessageCallback, nullptr);

            // Bind empty vertex array object to avoid crash
            auto emptyVAO = 0u;

            glGenVertexArrays(1, &emptyVAO);
            glBindVertexArray(emptyVAO);

            m_EnableReverseZ = true;

            // Create buffers
            auto cameraBuffer = std::shared_ptr<Buffer<GpuCamera>>(new Buffer<GpuCamera>());

            // Create samplers
            auto samplerShadowBorder = std::shared_ptr<Sampler>(new Sampler());
            samplerShadowBorder->SetParameter(GL_TEXTURE_BORDER_COLOR, glm::vec4(1.f));
            samplerShadowBorder->SetParameter(GL_TEXTURE_COMPARE_MODE, static_cast<GLenum>(GL_COMPARE_REF_TO_TEXTURE));
            samplerShadowBorder->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            samplerShadowBorder->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            samplerShadowBorder->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            samplerShadowBorder->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            samplerShadowBorder->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_REPEAT));
            samplerShadowBorder->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_REPEAT));
            samplerShadowBorder->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_REPEAT));

            auto samplerWrap = std::shared_ptr<Sampler>(new Sampler());
            samplerWrap->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            samplerWrap->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            samplerWrap->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            samplerWrap->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            samplerWrap->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_REPEAT));
            samplerWrap->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_REPEAT));
            samplerWrap->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_REPEAT));

            // Create textures
            auto height = g_Window->m_ScreenHeight;
            auto width = g_Window->m_ScreenWidth;
            auto depthTexture = std::shared_ptr<Texture2D>(new Texture2D(width, height, 1, GL_DEPTH_COMPONENT32F));
            auto lightingTexture = std::shared_ptr<Texture2D>(new Texture2D(width, height, 1, GL_RGB16F));
            auto shadowCsmColorTexture2DArray = std::shared_ptr<Texture2DArray>(new Texture2DArray(SHADOW_SIZE, SHADOW_SIZE, 5, 1, GL_R32F));
            auto shadowCsmDepthTexture2DArray = std::shared_ptr<Texture2DArray>(new Texture2DArray(SHADOW_SIZE, SHADOW_SIZE, 5, 1, GL_DEPTH_COMPONENT32F));

            // Create framebuffers
            auto depthFramebuffer = std::shared_ptr<Framebuffer>(new Framebuffer());
            depthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, depthTexture);

            auto lightingFramebuffer = std::shared_ptr<Framebuffer>(new Framebuffer());
            lightingFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, lightingTexture);
            lightingFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, depthTexture);

            auto screenFramebuffer = std::shared_ptr<DefaultFramebuffer>(new DefaultFramebuffer());

            auto shadowCsmFramebuffer = std::shared_ptr<Framebuffer>(new Framebuffer());
            shadowCsmFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, shadowCsmColorTexture2DArray);
            shadowCsmFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, shadowCsmDepthTexture2DArray);

            // Create shader programs
            auto depthShaderProgram = std::shared_ptr<ShaderProgram>(new ShaderProgram());
            assert(depthShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/depth.vert"));

            auto lightingShaderProgram = std::shared_ptr<ShaderProgram>(new ShaderProgram());
            assert(lightingShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/lighting.vert"));
            assert(lightingShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/lighting.frag"));

            auto screenShaderProgram = std::shared_ptr<ShaderProgram>(new ShaderProgram());
            assert(screenShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/screen.vert"));
            assert(screenShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/screen.frag"));

            auto shadowCsmShaderProgram = std::shared_ptr<ShaderProgram>(new ShaderProgram());
            assert(shadowCsmShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/shadow_csm.vert"));
            assert(shadowCsmShaderProgram->Link(GL_GEOMETRY_SHADER, g_ResourcePath / "shaders/shadow_csm.geom"));
            assert(shadowCsmShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/shadow_csm.frag"));

            m_CameraBuffer = cameraBuffer;
            m_DepthFramebuffer = depthFramebuffer;
            m_DepthShaderProgram = depthShaderProgram;
            m_DepthTexture2D = depthTexture;
            m_LightingFramebuffer = lightingFramebuffer;
            m_LightingShaderProgram = lightingShaderProgram;
            m_LightingTexture2D = lightingTexture;
            m_SamplerWrap = samplerWrap;
            m_SamplerShadowBorder = samplerShadowBorder;
            m_ScreenFramebuffer = screenFramebuffer;
            m_ScreenShaderProgram = screenShaderProgram;
            m_ShadowCsmColorTexture2DArray = shadowCsmColorTexture2DArray;
            m_ShadowCsmDepthTexture2DArray = shadowCsmDepthTexture2DArray;
            m_ShadowCsmFramebuffer = shadowCsmFramebuffer;
            m_ShadowCsmShaderProgram = shadowCsmShaderProgram;
        } else {
            std::cout << "Can't initialize GLEW. " << glewGetErrorString(result) << std::endl;
        }
    } else {
        std::cout << "Context could not be created. " << SDL_GetError() << std::endl;
    }
}

Render::~Render() {
    SDL_GL_DeleteContext(m_Context);
}

void Render::LoadScene(const Scene &scene) {
    if (!m_Context) {
        return;
    }

    auto diffuseImages = std::vector<std::shared_ptr<Image>>();
    auto metalnessImages = std::vector<std::shared_ptr<Image>>();
    auto normalImages = std::vector<std::shared_ptr<Image>>();
    auto roughnessImages = std::vector<std::shared_ptr<Image>>();

    for (const auto &material : scene.m_Materials) {
        if (material.m_DiffuseImage) {
            diffuseImages.push_back(material.m_DiffuseImage);
        }
        if (material.m_MetalnessImage) {
            metalnessImages.push_back(material.m_MetalnessImage);
        }
        if (material.m_NormalImage) {
            normalImages.push_back(material.m_NormalImage);
        }
        if (material.m_RoughnessImage) {
            roughnessImages.push_back(material.m_RoughnessImage);
        }
    }

    // Load textures
    // Better use texture atlas (my GPU doesn't support bindless textures) than texture array
    auto diffuseTexture2DArray = std::shared_ptr<Texture2DArray>();
    auto metalnessTexture2DArray = std::shared_ptr<Texture2DArray>();
    auto normalTexture2DArray = std::shared_ptr<Texture2DArray>();
    auto roughnessTexture2DArray = std::shared_ptr<Texture2DArray>();

    if (diffuseImages.size() > 0) {
        diffuseTexture2DArray = std::shared_ptr<Texture2DArray>(new Texture2DArray(diffuseImages));
    }
    if (metalnessImages.size() > 0) {
        metalnessTexture2DArray = std::shared_ptr<Texture2DArray>(new Texture2DArray(metalnessImages));
    }
    if (normalImages.size() > 0) {
        normalTexture2DArray = std::shared_ptr<Texture2DArray>(new Texture2DArray(normalImages));
    }
    if (roughnessImages.size() > 0) {
        roughnessTexture2DArray = std::shared_ptr<Texture2DArray>(new Texture2DArray(roughnessImages));
    }

    // Load buffers
    auto materialBuffer = std::shared_ptr<Buffer<GpuMaterial>>(new Buffer<GpuMaterial>(scene.m_Materials.size()));

    auto materials = std::vector<GpuMaterial>();
    auto numDiffuseImages = 0u;
    auto numMetalnessImages = 0u;
    auto numNormalImages = 0u;
    auto numRoughnessImages = 0u;

    for (const auto &material : scene.m_Materials) {
        auto gpuMaterial = GpuMaterial {
            .m_DiffuseMap = material.m_DiffuseImage ? numDiffuseImages : -1,
            .m_MetalnessMap = material.m_MetalnessImage ? numDiffuseImages : -1,
            .m_NormalMap = material.m_NormalImage ? numNormalImages : -1,
            .m_RoughnessMap = material.m_RoughnessImage ? numRoughnessImages : -1,
        };

        numDiffuseImages += material.m_DiffuseImage ? 1 : 0;
        numMetalnessImages += material.m_MetalnessImage ? 1 : 0;
        numNormalImages += material.m_NormalImage ? 1 : 0;
        numRoughnessImages += material.m_RoughnessImage ? 1 : 0;
        
        materials.push_back(gpuMaterial);
    }

    materialBuffer->SetData(materials, 0);

    auto drawIndirectBuffer = std::shared_ptr<DrawIndirectBuffer>(new DrawIndirectBuffer(scene.m_Meshes.size()));
    auto indexBuffer = std::shared_ptr<Buffer<GpuIndex>>(new Buffer<GpuIndex>(scene.NumIndices()));
    auto lightEnvironmentBuffer = std::shared_ptr<Buffer<GpuLightEnvironment>>(new Buffer<GpuLightEnvironment>());
    auto vertexBuffer = std::shared_ptr<Buffer<GpuVertex>>(new Buffer<GpuVertex>(scene.NumVertices()));

    auto meshes = std::vector<std::tuple<GLuint, GLuint>>();
    auto indexOffset = 0u;
    auto vertexOffset = 0u;

    meshes.reserve(m_Meshes.size());

    for (const auto &mesh : scene.m_Meshes) {
        auto drawIndirectCommand = DrawIndirectCommand {
            .m_NumVertices = static_cast<GLuint>(mesh.m_Indices.size()),
            .m_NumInstances = 1,
            .m_FirstVertex = indexOffset,
            .m_FirstInstance = static_cast<GLuint>(meshes.size()),
        };

        auto indices = std::vector<unsigned int>();

        indices.reserve(mesh.m_Indices.size());

        for (const auto &index : mesh.m_Indices) {
            indices.push_back(vertexOffset + index);
        }

        auto &vertices = mesh.m_Vertices;

        drawIndirectBuffer->SetData(drawIndirectCommand, meshes.size());
        indexBuffer->SetData(indices, indexOffset);
        vertexBuffer->SetData(vertices, vertexOffset);

        indexOffset += indices.size();
        vertexOffset += vertices.size();

        meshes.push_back(std::make_tuple(indexOffset, indices.size()));
    }

    m_DiffuseTexture2DArray = diffuseTexture2DArray;
    m_DrawIndirectBuffer = drawIndirectBuffer;
    m_IndexBuffer = indexBuffer;
    m_LightEnvironmentBuffer = lightEnvironmentBuffer;
    m_MaterialBuffer = materialBuffer;
    m_Meshes = meshes;
    m_MetalnessTexture2DArray = metalnessTexture2DArray;
    m_NormalTexture2DArray = normalTexture2DArray;
    m_RoughnessTexture2DArray = roughnessTexture2DArray;
    m_VertexBuffer = vertexBuffer;
}

void Render::Update() {
    if (!m_Context) {
        return;
    }

    // Update camera
    assert(g_Camera);

    auto cameraProjection = g_Camera->Projection(m_EnableReverseZ);
    auto cameraView = g_Camera->View();
    auto gpuCamera = GpuCamera {
        .m_Projection = cameraProjection,
        .m_View = cameraView,
        .m_Position = g_Camera->m_Position,
        .m_FarZ = g_Camera->m_FarZ,
        .m_NearZ = g_Camera->m_NearZ,
    };

    m_CameraBuffer->SetData(gpuCamera, 0);

    if (g_LightEnvironment && m_LightEnvironmentBuffer) {
        auto cascadeLevels = std::array<float, 4> {
            g_Camera->m_FarZ * 1.f / 80.f,
            g_Camera->m_FarZ * 1.f / 40.f,
            g_Camera->m_FarZ * 1.f / 20.f,
            g_Camera->m_FarZ * 1.f / 10.f,
        };
        auto gpuLightEnvironment = GpuLightEnvironment {
            .m_CascadeViewProjections = g_LightEnvironment->CascadeViewProjections(*g_Camera, cascadeLevels, m_EnableReverseZ),
            .m_CascadePlaneDistances = cascadeLevels,
            .m_AmbientColor = g_LightEnvironment->m_AmbientColor,
            .m_BaseColor = g_LightEnvironment->m_BaseColor,
            .m_Direction = g_LightEnvironment->Forward(),
        };

        m_LightEnvironmentBuffer->SetData(gpuLightEnvironment, 0);
    }

    m_SamplerShadowBorder->SetParameter(GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(m_EnableReverseZ ? GL_GEQUAL : GL_LEQUAL));

    // Draw scene
    ShadowCsmPass();
    DepthPass();
    LightingPass();
    ScreenPass();
}

void Render::ShadowCsmPass() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glDepthFunc(m_EnableReverseZ ? GL_GEQUAL : GL_LEQUAL);
    glDepthMask(true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    glScissor(0, 0, SHADOW_SIZE, SHADOW_SIZE);
    glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE);

    assert(m_ShadowCsmFramebuffer);
    assert(m_ShadowCsmShaderProgram);

    m_ShadowCsmFramebuffer->Bind();
    m_ShadowCsmFramebuffer->ClearColor(0, 0.f, 0.f, 0.f, 1.f);
    m_ShadowCsmFramebuffer->ClearDepth(0, m_EnableReverseZ ? 0.f : 1.f);

    if (m_IndexBuffer) {
        m_IndexBuffer->Bind(0);
    }
    if (m_LightEnvironmentBuffer) {
        m_LightEnvironmentBuffer->Bind(1);
    }
    if (m_VertexBuffer) {
        m_VertexBuffer->Bind(2);
    }

    m_ShadowCsmShaderProgram->Use();

    if (m_DrawIndirectBuffer) {
        m_DrawIndirectBuffer->Bind();

        glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
    }
}

void Render::DepthPass() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glDepthFunc(m_EnableReverseZ ? GL_GEQUAL : GL_LEQUAL);
    glDepthMask(true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    glScissor(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);
    glViewport(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);

    assert(m_DepthFramebuffer);
    assert(m_DepthShaderProgram);

    m_DepthFramebuffer->Bind();
    m_DepthFramebuffer->ClearDepth(0, m_EnableReverseZ ? 0.f : 1.f);

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }
    if (m_IndexBuffer) {
        m_IndexBuffer->Bind(1);
    }
    if (m_VertexBuffer) {
        m_VertexBuffer->Bind(2);
    }

    m_DepthShaderProgram->Use();

    if (m_DrawIndirectBuffer) {
        m_DrawIndirectBuffer->Bind();

        glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
    }
}

void Render::LightingPass() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glDepthFunc(GL_EQUAL);
    glDepthMask(false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    glScissor(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);
    glViewport(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);

    assert(m_LightingFramebuffer);
    assert(m_LightingShaderProgram);

    m_LightingFramebuffer->Bind();
    m_LightingFramebuffer->ClearColor(0, 0.f, 0.f, 0.f, 1.f);

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }
    if (m_IndexBuffer) {
        m_IndexBuffer->Bind(1);
    }
    if (m_LightEnvironmentBuffer) {
        m_LightEnvironmentBuffer->Bind(2);
    }
    if (m_MaterialBuffer) {
        m_MaterialBuffer->Bind(3);
    }
    if (m_VertexBuffer) {
        m_VertexBuffer->Bind(4);
    }

    if (m_DiffuseTexture2DArray) {
        m_DiffuseTexture2DArray->Bind(0, *m_SamplerWrap);
    }
    if (m_MetalnessTexture2DArray) {
        m_MetalnessTexture2DArray->Bind(1, *m_SamplerWrap);
    }
    if (m_NormalTexture2DArray) {
        m_NormalTexture2DArray->Bind(2, *m_SamplerWrap);
    }
    if (m_RoughnessTexture2DArray) {
        m_RoughnessTexture2DArray->Bind(3, *m_SamplerWrap);
    }
    if (m_ShadowCsmColorTexture2DArray) {
        m_ShadowCsmColorTexture2DArray->Bind(4, *m_SamplerShadowBorder);
    }
    if (m_ShadowCsmDepthTexture2DArray) {
        m_ShadowCsmDepthTexture2DArray->Bind(5, *m_SamplerShadowBorder);
    }

    m_LightingShaderProgram->Use();

    glUniform1i(0, m_EnableReverseZ);
    glUniform1f(1, 1.f / SHADOW_SIZE);

    if (m_DrawIndirectBuffer) {
        m_DrawIndirectBuffer->Bind();

        glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
    }
}

void Render::ScreenPass() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glScissor(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);
    glViewport(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);

    assert(m_LightingTexture2D);
    assert(m_ScreenFramebuffer);
    assert(m_ScreenShaderProgram);

    m_ScreenFramebuffer->Bind();

    m_LightingTexture2D->Bind(0);

    m_ScreenShaderProgram->Use();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}