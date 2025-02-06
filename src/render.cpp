#include <array>
#include <iostream>

#include <glm/ext/matrix_transform.hpp>
#include <random>

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

    SDL_GL_SetSwapInterval(-1);

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

            m_EnableAmbientOcclusion = true;
            m_EnableReverseZ = true;
            m_NumFrames = 0;

            // Create buffers
            auto cameraBuffer = std::make_shared<Buffer<GpuCamera>>();

            // Create samplers
            auto samplerBorderWhite = std::make_shared<Sampler>();
            samplerBorderWhite->SetParameter(GL_TEXTURE_BORDER_COLOR, glm::vec4(1.f));
            samplerBorderWhite->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            samplerBorderWhite->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            samplerBorderWhite->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            samplerBorderWhite->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            samplerBorderWhite->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_REPEAT));
            samplerBorderWhite->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_REPEAT));
            samplerBorderWhite->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_REPEAT));

            auto samplerClamp = std::make_shared<Sampler>();
            samplerClamp->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            samplerClamp->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            samplerClamp->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            samplerClamp->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            samplerClamp->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_CLAMP_TO_EDGE));
            samplerClamp->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_CLAMP_TO_EDGE));
            samplerClamp->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_CLAMP_TO_EDGE));

            auto samplerWrap = std::make_shared<Sampler>();
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
            auto minHeight = height;
            auto minWidth = width;
            auto mipLevel = 0u;

            for (auto i = 0u; minHeight > 0 && minWidth > 0; i++) {
                mipLevel++;
                minHeight /= 2;
                minWidth /= 2;
            }

            auto ambientOcclusionTexture2D = std::make_shared<Texture2D>(width, height , 1, GL_R16F);
            auto ambientOcclusionSpartialTexture2D = std::make_shared<Texture2D>(width, height, 1, GL_R16F);
            auto ambientOcclusionTemporalTexture2D = std::make_shared<Texture2D>(width, height, 1, GL_R16F);
            auto depthTexture2D = std::make_shared<Texture2D>(width, height, mipLevel, GL_DEPTH_COMPONENT32F);
            auto lastAmbientOcclusionTemporalTexture2D = std::make_shared<Texture2D>(width, height, 1, GL_R16F);
            auto lastDepthTexture2D = std::make_shared<Texture2D>(width, height, mipLevel, GL_DEPTH_COMPONENT32F);
            auto lightingTexture2D = std::make_shared<Texture2D>(width, height, 1, GL_RGBA16F);
            auto shadowCsmColorTexture2DArray = std::make_shared<Texture2DArray>(SHADOW_SIZE, SHADOW_SIZE, 5, 1, GL_R32F);
            auto shadowCsmDepthTexture2DArray = std::make_shared<Texture2DArray>(SHADOW_SIZE, SHADOW_SIZE, 5, 1, GL_DEPTH_COMPONENT32F);
            auto velocityTexture2D = std::make_shared<Texture2D>(width / 2, height / 2, 1, GL_RG16F);

            auto depthTextureView2Ds = std::vector<std::shared_ptr<TextureView2D>>();
            auto lastDepthTextureView2Ds = std::vector<std::shared_ptr<TextureView2D>>();

            for (auto i = 0u; i < mipLevel; i++) {
                depthTextureView2Ds.push_back(std::make_shared<TextureView2D>(depthTexture2D, i, mipLevel - i));
                lastDepthTextureView2Ds.push_back(std::make_shared<TextureView2D>(lastDepthTexture2D, i, mipLevel - i));
            }

            // Create framebuffers
            auto ambientOcclusionFramebuffer = std::make_shared<Framebuffer>();
            ambientOcclusionFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, ambientOcclusionTexture2D);

            auto ambientOcclusionSpartialFramebuffer = std::make_shared<Framebuffer>();
            ambientOcclusionSpartialFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, ambientOcclusionSpartialTexture2D);

            auto ambientOcclusionTemporalFramebuffer = std::make_shared<Framebuffer>();
            ambientOcclusionTemporalFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, ambientOcclusionTemporalTexture2D);

            auto depthFramebuffer = std::make_shared<Framebuffer>();
            depthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, depthTexture2D);

            auto downsampleDepthFramebuffer = std::make_shared<Framebuffer>();
            downsampleDepthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, depthTextureView2Ds[0]);

            auto lastAmbientOcclusionTemporalFramebuffer = std::make_shared<Framebuffer>();
            lastAmbientOcclusionTemporalFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, lastAmbientOcclusionTemporalTexture2D);

            auto lastDepthFramebuffer = std::make_shared<Framebuffer>();
            lastDepthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, lastDepthTexture2D);

            auto lastLightingFramebuffer = std::make_shared<Framebuffer>();
            lastLightingFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, lightingTexture2D);
            lastLightingFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, lastDepthTexture2D);

            auto lightingFramebuffer = std::make_shared<Framebuffer>();
            lightingFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, lightingTexture2D);
            lightingFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, depthTexture2D);

            auto screenFramebuffer = std::make_shared<DefaultFramebuffer>();

            auto shadowCsmFramebuffer = std::make_shared<Framebuffer>();
            shadowCsmFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, shadowCsmColorTexture2DArray);
            shadowCsmFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, shadowCsmDepthTexture2DArray);

            auto velocityFramebuffer = std::make_shared<Framebuffer>();
            velocityFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, velocityTexture2D);

            // Create shader programs
            auto ambientOcclusionShaderProgram = std::make_shared<ShaderProgram>();
            assert(ambientOcclusionShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/gtao.vert"));
            assert(ambientOcclusionShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/gtao.frag"));
            
            auto ambientOcclusionSpartialShaderProgram = std::make_shared<ShaderProgram>();
            assert(ambientOcclusionSpartialShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/gtao_spartial.vert"));
            assert(ambientOcclusionSpartialShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/gtao_spartial.frag"));

            auto ambientOcclusionTemporalShaderProgram = std::make_shared<ShaderProgram>();
            assert(ambientOcclusionTemporalShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/gtao_temporal.vert"));
            assert(ambientOcclusionTemporalShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/gtao_temporal.frag"));

            auto depthShaderProgram = std::make_shared<ShaderProgram>();
            assert(depthShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/depth.vert"));

            auto downsampleDepthShaderProgram = std::make_shared<ShaderProgram>();
            assert(downsampleDepthShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/downsample_depth.vert"));
            assert(downsampleDepthShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/downsample_depth.frag"));

            auto lightingShaderProgram = std::make_shared<ShaderProgram>();
            assert(lightingShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/lighting.vert"));
            assert(lightingShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/lighting.frag"));

            auto screenShaderProgram = std::make_shared<ShaderProgram>();
            assert(screenShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/screen.vert"));
            assert(screenShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/screen.frag"));

            auto shadowCsmShaderProgram = std::make_shared<ShaderProgram>();
            assert(shadowCsmShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/shadow_csm.vert"));
            assert(shadowCsmShaderProgram->Link(GL_GEOMETRY_SHADER, g_ResourcePath / "shaders/shadow_csm.geom"));
            assert(shadowCsmShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/shadow_csm.frag"));

            auto velocityShaderProgram = std::make_shared<ShaderProgram>();
            assert(velocityShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/velocity.vert"));
            assert(velocityShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/velocity.frag"));

            m_AmbientOcclusionFramebuffer = ambientOcclusionFramebuffer;
            m_AmbientOcclusionShaderProgram = ambientOcclusionShaderProgram;
            m_AmbientOcclusionTexture2D = ambientOcclusionTexture2D;
            m_AmbientOcclusionSpartialFramebuffer = ambientOcclusionSpartialFramebuffer;
            m_AmbientOcclusionSpartialShaderProgram = ambientOcclusionSpartialShaderProgram;
            m_AmbientOcclusionSpartialTexture2D = ambientOcclusionSpartialTexture2D;
            m_AmbientOcclusionTemporalFramebuffer = ambientOcclusionTemporalFramebuffer;
            m_AmbientOcclusionTemporalShaderProgram = ambientOcclusionTemporalShaderProgram;
            m_AmbientOcclusionTemporalTexture2D = ambientOcclusionTemporalTexture2D;
            m_CameraBuffer = cameraBuffer;
            m_DepthFramebuffer = depthFramebuffer;
            m_DepthShaderProgram = depthShaderProgram;
            m_DepthTexture2D = depthTexture2D;
            m_DepthTextureView2Ds = depthTextureView2Ds;
            m_DownsampleDepthFramebuffer = downsampleDepthFramebuffer;
            m_DownsampleDepthShaderProgram = downsampleDepthShaderProgram;
            m_LastAmbientOcclusionTemporalFramebuffer = lastAmbientOcclusionTemporalFramebuffer;
            m_LastAmbientOcclusionTemporalTexture2D = lastAmbientOcclusionTemporalTexture2D;
            m_LastDepthFramebuffer = lastDepthFramebuffer;
            m_LastDepthTexture2D = lastDepthTexture2D;
            m_LastDepthTextureView2Ds = lastDepthTextureView2Ds;
            m_LastLightingFramebuffer = lastLightingFramebuffer;
            m_LightingFramebuffer = lightingFramebuffer;
            m_LightingShaderProgram = lightingShaderProgram;
            m_LightingTexture2D = lightingTexture2D;
            m_SamplerBorderWhite = samplerBorderWhite;
            m_SamplerClamp = samplerClamp;
            m_SamplerWrap = samplerWrap;
            m_ScreenFramebuffer = screenFramebuffer;
            m_ScreenShaderProgram = screenShaderProgram;
            m_ShadowCsmColorTexture2DArray = shadowCsmColorTexture2DArray;
            m_ShadowCsmDepthTexture2DArray = shadowCsmDepthTexture2DArray;
            m_ShadowCsmFramebuffer = shadowCsmFramebuffer;
            m_ShadowCsmShaderProgram = shadowCsmShaderProgram;
            m_VelocityFramebuffer = velocityFramebuffer;
            m_VelocityShaderProgram = velocityShaderProgram;
            m_VelocityTexture2D = velocityTexture2D;
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

void Render::LoadModel(const Model &model) {
    if (!m_Context) {
        return;
    }

    auto diffuseImages = std::vector<std::shared_ptr<Image>>();
    auto metalnessImages = std::vector<std::shared_ptr<Image>>();
    auto normalImages = std::vector<std::shared_ptr<Image>>();
    auto roughnessImages = std::vector<std::shared_ptr<Image>>();

    for (const auto &material : model.m_Materials) {
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
        diffuseTexture2DArray = std::make_shared<Texture2DArray>(diffuseImages);
    }
    if (metalnessImages.size() > 0) {
        metalnessTexture2DArray = std::make_shared<Texture2DArray>(metalnessImages);
    }
    if (normalImages.size() > 0) {
        normalTexture2DArray = std::make_shared<Texture2DArray>(normalImages);
    }
    if (roughnessImages.size() > 0) {
        roughnessTexture2DArray = std::make_shared<Texture2DArray>(roughnessImages);
    }

    // Load buffers
    auto materialBuffer = std::make_shared<Buffer<GpuMaterial>>(model.m_Materials.size());

    auto materials = std::vector<GpuMaterial>();
    auto numDiffuseImages = 0u;
    auto numMetalnessImages = 0u;
    auto numNormalImages = 0u;
    auto numRoughnessImages = 0u;

    for (const auto &material : model.m_Materials) {
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

    auto drawIndirectBuffer = std::make_shared<DrawIndirectBuffer>(model.m_Meshes.size());
    auto indexBuffer = std::make_shared<Buffer<GpuIndex>>(model.NumIndices());
    auto lightEnvironmentBuffer = std::make_shared<Buffer<GpuLightEnvironment>>();
    auto vertexBuffer = std::make_shared<Buffer<GpuVertex>>(model.NumVertices());

    auto meshes = std::vector<std::tuple<GLuint, GLuint>>();
    auto indexOffset = 0u;
    auto vertexOffset = 0u;

    meshes.reserve(m_Meshes.size());

    for (const auto &mesh : model.m_Meshes) {
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
    auto cameraFovY = glm::radians(g_Camera->m_FovY);
    auto cameraHalfFovY = cameraFovY * 0.5f;
    auto cameraV = glm::tan(cameraHalfFovY);
    auto cameraH = g_Camera->m_AspectRatio * cameraV;
    auto cameraHalfFovX = glm::atan(cameraH);
    auto cameraFovX = cameraHalfFovX * 2.f;
    auto cameraView = g_Camera->View();

    auto gpuCamera = GpuCamera {
        .m_Projection = cameraProjection,
        .m_ProjectionInversed = glm::inverse(cameraProjection),
        .m_View = cameraView,
        .m_Position = g_Camera->m_Position,
        .m_FarZ = g_Camera->m_FarZ,
        .m_NearZ = g_Camera->m_NearZ,
        .m_FovX = cameraFovX,
        .m_FovY = cameraFovY,
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

    std::swap(m_AmbientOcclusionTemporalFramebuffer, m_LastAmbientOcclusionTemporalFramebuffer);
    std::swap(m_AmbientOcclusionTemporalTexture2D, m_LastAmbientOcclusionTemporalTexture2D);
    std::swap(m_DepthFramebuffer, m_LastDepthFramebuffer);
    std::swap(m_DepthTexture2D, m_LastDepthTexture2D);
    std::swap(m_DepthTextureView2Ds, m_LastDepthTextureView2Ds);
    std::swap(m_LightingFramebuffer, m_LastLightingFramebuffer);

    // Draw model
    ShadowCsmPass();
    DepthPass();
    DownsampleDepthPass();
    VelocityPass();
    AmbientOcclusionPass();
    AmbientOcclusionSpartialPass();
    AmbientOcclusionTemporalPass();
    LightingPass();
    ScreenPass();

    m_NumFrames++;
}

void Render::ShadowCsmPass() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glDepthFunc(m_EnableReverseZ ? GL_GEQUAL : GL_LEQUAL);
    glDepthMask(true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    glScissor(0, 0, m_ShadowCsmColorTexture2DArray->m_Width, m_ShadowCsmColorTexture2DArray->m_Height);
    glViewport(0, 0, m_ShadowCsmColorTexture2DArray->m_Width, m_ShadowCsmColorTexture2DArray->m_Height);

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

    glScissor(0, 0, m_DepthTexture2D->m_Width, m_DepthTexture2D->m_Height);
    glViewport(0, 0, m_DepthTexture2D->m_Width, m_DepthTexture2D->m_Height);

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

void Render::DownsampleDepthPass() {
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_ALWAYS);
    glDepthMask(true);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    assert(m_DepthTexture2D);
    assert(m_DownsampleDepthFramebuffer);
    assert(m_DownsampleDepthShaderProgram);

    m_DownsampleDepthFramebuffer->Bind();
    m_DownsampleDepthShaderProgram->Use();

    glUniform1i(0, m_EnableReverseZ);

    auto height = m_DepthTexture2D->m_Height;
    auto width = m_DepthTexture2D->m_Width;

    for (auto i = 0u; i < m_DepthTexture2D->m_MipLevel - 1; i++) {
        width /= 2;
        height /= 2;

        glScissor(0, 0, width, height);
        glViewport(0, 0, width, height);

        m_DownsampleDepthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_DepthTextureView2Ds.at(i + 1));

        m_DepthTextureView2Ds.at(i + 0)->Bind(0, m_SamplerClamp);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Render::VelocityPass() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    assert(m_VelocityFramebuffer);
    assert(m_VelocityShaderProgram);
    assert(m_VelocityTexture2D);

    glScissor(0, 0, m_VelocityTexture2D->m_Width, m_VelocityTexture2D->m_Height);
    glViewport(0, 0, m_VelocityTexture2D->m_Width, m_VelocityTexture2D->m_Height);

    m_VelocityFramebuffer->Bind();

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    m_DepthTextureView2Ds.at(1)->Bind(0, m_SamplerClamp);
    m_LastDepthTextureView2Ds.at(1)->Bind(1, m_SamplerClamp);

    m_VelocityShaderProgram->Use();

    auto rd = std::random_device();
    auto gen = std::mt19937(rd());
    auto dist = std::uniform_real_distribution<>();

    auto jitter0 = glm::vec2(dist(gen) * 1.f / m_VelocityTexture2D->m_Width, dist(gen) * 1.f / m_VelocityTexture2D->m_Height);
    auto jitter1 = glm::vec2(dist(gen) * 1.f / m_VelocityTexture2D->m_Width, dist(gen) * 1.f / m_VelocityTexture2D->m_Height);

    glUniform2fv(0, 1, reinterpret_cast<float *>(&jitter0));
    glUniform2fv(1, 1, reinterpret_cast<float *>(&jitter1));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionPass() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    assert(m_AmbientOcclusionFramebuffer);
    assert(m_AmbientOcclusionShaderProgram);
    assert(m_AmbientOcclusionTexture2D);

    glScissor(0, 0, m_AmbientOcclusionTexture2D->m_Width, m_AmbientOcclusionTexture2D->m_Height);
    glViewport(0, 0, m_AmbientOcclusionTexture2D->m_Width, m_AmbientOcclusionTexture2D->m_Height);

    m_AmbientOcclusionFramebuffer->Bind();

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    m_DepthTextureView2Ds.at(1)->Bind(0, m_SamplerClamp);

    m_AmbientOcclusionShaderProgram->Use();

    auto screenSize = glm::vec2(m_AmbientOcclusionTexture2D->m_Width, m_AmbientOcclusionTexture2D->m_Height);
    auto screenSizeInv = glm::vec2(1.f / m_AmbientOcclusionTexture2D->m_Width, 1.f / m_AmbientOcclusionTexture2D->m_Height);

    const float rotations[] = { 60.f, 300.f, 180.f, 240.f, 120.f, 0.f };

    glUniform2fv(0, 1, reinterpret_cast<float *>(&screenSize));
    glUniform2fv(1, 1, reinterpret_cast<float *>(&screenSizeInv));
    glUniform1f(2, 4.f);
    glUniform1f(3, rotations[m_NumFrames % 6] / 360.f * 2.f * 3.14159265358979323846f);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionSpartialPass() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    assert(m_AmbientOcclusionSpartialFramebuffer);
    assert(m_AmbientOcclusionSpartialShaderProgram);
    assert(m_AmbientOcclusionSpartialTexture2D);

    glScissor(0, 0, m_AmbientOcclusionSpartialTexture2D->m_Width, m_AmbientOcclusionSpartialTexture2D->m_Height);
    glViewport(0, 0, m_AmbientOcclusionSpartialTexture2D->m_Width, m_AmbientOcclusionSpartialTexture2D->m_Height);

    m_AmbientOcclusionSpartialFramebuffer->Bind();

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    assert(m_AmbientOcclusionTexture2D);
    assert(m_DepthTexture2D);

    m_AmbientOcclusionTexture2D->Bind(0, m_SamplerClamp);
    m_DepthTextureView2Ds.at(1)->Bind(1, m_SamplerClamp);

    m_AmbientOcclusionSpartialShaderProgram->Use();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionTemporalPass() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    assert(m_AmbientOcclusionTemporalFramebuffer);
    assert(m_AmbientOcclusionTemporalShaderProgram);
    assert(m_AmbientOcclusionTemporalTexture2D);

    glScissor(0, 0, m_AmbientOcclusionTemporalTexture2D->m_Width, m_AmbientOcclusionTemporalTexture2D->m_Height);
    glViewport(0, 0, m_AmbientOcclusionTemporalTexture2D->m_Width, m_AmbientOcclusionTemporalTexture2D->m_Height);

    m_AmbientOcclusionTemporalFramebuffer->Bind();

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    assert(m_AmbientOcclusionSpartialTexture2D);
    assert(m_LastAmbientOcclusionTemporalTexture2D);
    assert(m_VelocityTexture2D);

    m_AmbientOcclusionSpartialTexture2D->Bind(0, m_SamplerClamp);
    m_DepthTextureView2Ds.at(1)->Bind(1, m_SamplerClamp);
    m_LastAmbientOcclusionTemporalTexture2D->Bind(2, m_SamplerClamp);
    m_LastDepthTextureView2Ds.at(1)->Bind(3, m_SamplerClamp);
    m_VelocityTexture2D->Bind(4, m_SamplerClamp);

    m_AmbientOcclusionTemporalShaderProgram->Use();

    auto screenSize = glm::vec2(m_AmbientOcclusionTemporalTexture2D->m_Width, m_AmbientOcclusionTemporalTexture2D->m_Height);

    glUniform1f(0, 0.7f);
    glUniform2fv(1, 1, reinterpret_cast<float *>(&screenSize));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::LightingPass() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glDepthFunc(GL_EQUAL);
    glDepthMask(false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);

    glScissor(0, 0, m_LightingTexture2D->m_Width, m_LightingTexture2D->m_Height);
    glViewport(0, 0, m_LightingTexture2D->m_Width, m_LightingTexture2D->m_Height);

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

    assert(m_AmbientOcclusionSpartialTexture2D);

    m_AmbientOcclusionSpartialTexture2D->Bind(0, m_SamplerClamp);

    if (m_DiffuseTexture2DArray) {
        m_DiffuseTexture2DArray->Bind(1, m_SamplerWrap);
    }
    if (m_MetalnessTexture2DArray) {
        m_MetalnessTexture2DArray->Bind(2, m_SamplerWrap);
    }
    if (m_NormalTexture2DArray) {
        m_NormalTexture2DArray->Bind(3, m_SamplerWrap);
    }
    if (m_RoughnessTexture2DArray) {
        m_RoughnessTexture2DArray->Bind(4, m_SamplerWrap);
    }

    assert(m_ShadowCsmColorTexture2DArray);
    assert(m_ShadowCsmDepthTexture2DArray);

    m_ShadowCsmColorTexture2DArray->Bind(5, m_SamplerBorderWhite);
    m_ShadowCsmDepthTexture2DArray->Bind(6, m_SamplerBorderWhite);

    m_LightingShaderProgram->Use();

    glUniform1i(0, m_EnableAmbientOcclusion);
    glUniform1i(1, m_EnableReverseZ);
    glUniform1f(2, 1.f / SHADOW_SIZE);

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

    m_LightingTexture2D->Bind(0, m_SamplerClamp);

    m_ScreenShaderProgram->Use();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}