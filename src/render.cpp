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
constexpr size_t MAX_NUM_LIGHTPOINTS = 1024;
constexpr GLuint SHADOW_CSM_SIZE = 2048;
constexpr GLuint SHADOW_CUBE_SIZE = 1024;

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
            auto shadowCsmColorTexture2DArray = std::make_shared<Texture2DArray>(SHADOW_CSM_SIZE, SHADOW_CSM_SIZE, 5, 1, GL_R32F);
            auto shadowCsmDepthTexture2DArray = std::make_shared<Texture2DArray>(SHADOW_CSM_SIZE, SHADOW_CSM_SIZE, 5, 1, GL_DEPTH_COMPONENT32F);

            auto depthTextureView2Ds = std::vector<std::shared_ptr<TextureView2D>>();
            auto lastDepthTextureView2Ds = std::vector<std::shared_ptr<TextureView2D>>();

            for (auto i = 0u; i < mipLevel; i++) {
                depthTextureView2Ds.push_back(std::make_shared<TextureView2D>(depthTexture2D, i, mipLevel - i, 0));
                lastDepthTextureView2Ds.push_back(std::make_shared<TextureView2D>(lastDepthTexture2D, i, mipLevel - i, 0));
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

            auto shadowCubeFramebuffer = std::make_shared<Framebuffer>();

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

            auto shadowCubeShaderProgram = std::make_shared<ShaderProgram>();
            assert(shadowCubeShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders/shadow_cube.vert"));
            assert(shadowCubeShaderProgram->Link(GL_GEOMETRY_SHADER, g_ResourcePath / "shaders/shadow_cube.geom"));
            assert(shadowCubeShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders/shadow_cube.frag"));

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
            m_ShadowCsmFilterRadius = 2.f;
            m_ShadowCsmFramebuffer = shadowCsmFramebuffer;
            m_ShadowCsmShaderProgram = shadowCsmShaderProgram;
            m_ShadowCubeFilterRadius = 2.f;
            m_ShadowCubeFramebuffer = shadowCubeFramebuffer;
            m_ShadowCubeShaderProgram = shadowCubeShaderProgram;
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
    auto lightPointBuffer = std::make_shared<Buffer<GpuLightPoint>>(MAX_NUM_LIGHTPOINTS);
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
    m_LightPointBuffer = lightPointBuffer;
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

    static glm::mat4 cameraLastView = glm::identity<glm::mat4>();

    auto cameraProjection = g_Camera->Projection(m_EnableReverseZ);
    auto cameraFovY = glm::radians(g_Camera->m_FovY);
    auto cameraHalfFovY = cameraFovY * 0.5f;
    auto cameraV = glm::tan(cameraHalfFovY);
    auto cameraH = g_Camera->m_AspectRatio * cameraV;
    auto cameraHalfFovX = glm::atan(cameraH);
    auto cameraFovX = cameraHalfFovX * 2.f;
    auto cameraView = g_Camera->View();

    auto gpuCamera = GpuCamera {
        .m_LastView = cameraLastView,
        .m_Projection = cameraProjection,
        .m_ProjectionInversed = glm::inverse(cameraProjection),
        .m_View = cameraView,
        .m_Position = g_Camera->m_Position,
        .m_FarZ = g_Camera->m_FarZ,
        .m_NearZ = g_Camera->m_NearZ,
        .m_FovX = cameraFovX,
        .m_FovY = cameraFovY,
    };

    cameraLastView = std::move(cameraView);

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

    if (m_LightPointBuffer) {
        for (auto i = 0u; i < std::min(g_LightPoints.size(), MAX_NUM_LIGHTPOINTS); i++) {
            const auto &lightPoint = g_LightPoints[i];

            const auto gpuLightPoint = GpuLightPoint {
                .m_ViewProjections = lightPoint->ViewProjections(m_EnableReverseZ),
                .m_Position = lightPoint->m_Position,
                .m_Radius = lightPoint->m_Radius,
                .m_BaseColor = lightPoint->m_BaseColor,
            };

            m_LightPointBuffer->SetData(gpuLightPoint, i);
        }
    }

    if (!m_ShadowCubeColorTextureCubeArray || m_ShadowCubeColorTextureCubeArray->m_Depth != g_LightPoints.size() * 6) {
        const auto layers = std::max(g_LightPoints.size() * 6lu, 6lu);
        const auto size = SHADOW_CUBE_SIZE;

        m_ShadowCubeColorTextureCubeArray = std::make_shared<TextureCubeArray>(size, size, layers, 1, GL_R32F);
        m_ShadowCubeColorTextureViewCubes.clear();
    
        while (m_ShadowCubeColorTextureViewCubes.size() < g_LightPoints.size()) {
            m_ShadowCubeColorTextureViewCubes.push_back(
                std::make_shared<TextureViewCube>(
                    m_ShadowCubeColorTextureCubeArray, 
                    0, 
                    1, 
                    m_ShadowCubeColorTextureViewCubes.size() * 6
                )
            );
        }
    }

    if (!m_ShadowCubeDepthTextureCubeArray || m_ShadowCubeDepthTextureCubeArray->m_Depth != g_LightPoints.size() * 6) {
        const auto layers = std::max(g_LightPoints.size() * 6lu, 6lu);
        const auto size = SHADOW_CUBE_SIZE;

        m_ShadowCubeDepthTextureCubeArray = std::make_shared<TextureCubeArray>(size, size, layers, 1, GL_DEPTH_COMPONENT32F);
        m_ShadowCubeDepthTextureViewCubes.clear();

        while (m_ShadowCubeDepthTextureViewCubes.size() < g_LightPoints.size()) {
            m_ShadowCubeDepthTextureViewCubes.push_back(
                std::make_shared<TextureViewCube>(
                    m_ShadowCubeDepthTextureCubeArray, 
                    0, 
                    1, 
                    m_ShadowCubeDepthTextureViewCubes.size() * 6
                )
            );
        }
    }

    std::swap(m_AmbientOcclusionTemporalTexture2D, m_LastAmbientOcclusionTemporalTexture2D);
    std::swap(m_DepthFramebuffer, m_LastDepthFramebuffer);
    std::swap(m_DepthTexture2D, m_LastDepthTexture2D);
    std::swap(m_DepthTextureView2Ds, m_LastDepthTextureView2Ds);
    std::swap(m_LightingFramebuffer, m_LastLightingFramebuffer);

    // Draw model
    ShadowCsmPass();
    ShadowCubePass();
    DepthPass();
    DownsampleDepthPass();
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

void Render::ShadowCubePass() {
    assert(m_ShadowCubeFramebuffer);
    assert(m_ShadowCubeShaderProgram);

    m_ShadowCubeFramebuffer->Bind();
    m_ShadowCubeShaderProgram->Use();

    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_ShadowCubeColorTextureCubeArray->m_Width, m_ShadowCubeColorTextureCubeArray->m_Height);
    glViewport(0, 0, m_ShadowCubeColorTextureCubeArray->m_Width, m_ShadowCubeColorTextureCubeArray->m_Height);

    if (m_IndexBuffer) {
        m_IndexBuffer->Bind(0);
    }
    if (m_LightPointBuffer) {
        m_LightPointBuffer->Bind(1);
    }
    if (m_VertexBuffer) {
        m_VertexBuffer->Bind(2);
    }

    for (auto i = 0u; i < m_ShadowCubeColorTextureViewCubes.size(); i++) {
        m_ShadowCubeFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_ShadowCubeColorTextureViewCubes.at(i));
        m_ShadowCubeFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_ShadowCubeDepthTextureViewCubes.at(i));
        m_ShadowCubeFramebuffer->ClearColor(0, 1.f, 1.f, 1.f, 1.f);
        m_ShadowCubeFramebuffer->ClearDepth(0, 1.f);

        glUniform1ui(0, i);

        if (m_DrawIndirectBuffer) {
            m_DrawIndirectBuffer->Bind();

            glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
        }
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

void Render::AmbientOcclusionPass() {
    assert(m_AmbientOcclusionFramebuffer);
    assert(m_AmbientOcclusionShaderProgram);
    assert(m_AmbientOcclusionTexture2D);

    m_AmbientOcclusionFramebuffer->Bind();
    m_AmbientOcclusionFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_AmbientOcclusionTexture2D);
    m_AmbientOcclusionShaderProgram->Use();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_AmbientOcclusionTexture2D->m_Width, m_AmbientOcclusionTexture2D->m_Height);
    glViewport(0, 0, m_AmbientOcclusionTexture2D->m_Width, m_AmbientOcclusionTexture2D->m_Height);

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    m_DepthTextureView2Ds.at(0)->Bind(0, m_SamplerClamp);

    auto screenSize = glm::vec2(m_AmbientOcclusionTexture2D->m_Width, m_AmbientOcclusionTexture2D->m_Height);
    auto screenSizeInv = glm::vec2(1.f / m_AmbientOcclusionTexture2D->m_Width, 1.f / m_AmbientOcclusionTexture2D->m_Height);

    constexpr std::array<float, 4> offsets = { 0.0f, 0.5f, 0.25f, 0.75f };
    constexpr std::array<float, 6> rotations = { 60.f, 300.f, 180.f, 240.f, 120.f, 0.f };

    glUniform1f(0, 1000.f);
    glUniform1f(1, 1.f);
    glUniform1i(2, 8);
    glUniform1f(3, offsets[m_NumFrames / 6 % offsets.size()]);
    glUniform1f(4, 8.f);
    glUniform1f(5, rotations[m_NumFrames % 6] / 360.f);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionSpartialPass() {
    assert(m_AmbientOcclusionSpartialFramebuffer);
    assert(m_AmbientOcclusionSpartialShaderProgram);
    assert(m_AmbientOcclusionSpartialTexture2D);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_AmbientOcclusionSpartialTexture2D->m_Width, m_AmbientOcclusionSpartialTexture2D->m_Height);
    glViewport(0, 0, m_AmbientOcclusionSpartialTexture2D->m_Width, m_AmbientOcclusionSpartialTexture2D->m_Height);

    m_AmbientOcclusionSpartialFramebuffer->Bind();
    m_AmbientOcclusionSpartialFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_AmbientOcclusionSpartialTexture2D);
    m_AmbientOcclusionSpartialShaderProgram->Use();

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    assert(m_AmbientOcclusionTexture2D);

    m_AmbientOcclusionTexture2D->Bind(0, m_SamplerClamp);
    m_DepthTextureView2Ds.at(1)->Bind(1, m_SamplerClamp);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionTemporalPass() {
    assert(m_AmbientOcclusionTemporalFramebuffer);
    assert(m_AmbientOcclusionTemporalShaderProgram);
    assert(m_AmbientOcclusionTemporalTexture2D);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_AmbientOcclusionTemporalTexture2D->m_Width, m_AmbientOcclusionTemporalTexture2D->m_Height);
    glViewport(0, 0, m_AmbientOcclusionTemporalTexture2D->m_Width, m_AmbientOcclusionTemporalTexture2D->m_Height);

    m_AmbientOcclusionTemporalFramebuffer->Bind();
    m_AmbientOcclusionTemporalFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_AmbientOcclusionTemporalTexture2D);
    m_AmbientOcclusionTemporalShaderProgram->Use();

    if (m_CameraBuffer) {
        m_CameraBuffer->Bind(0);
    }

    assert(m_AmbientOcclusionSpartialTexture2D);
    assert(m_LastAmbientOcclusionTemporalTexture2D);

    m_AmbientOcclusionSpartialTexture2D->Bind(0, m_SamplerClamp);
    m_DepthTextureView2Ds.at(1)->Bind(1, m_SamplerClamp);
    m_LastAmbientOcclusionTemporalTexture2D->Bind(2, m_SamplerClamp);
    m_LastDepthTextureView2Ds.at(1)->Bind(3, m_SamplerClamp);

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
    if (m_LightPointBuffer) {
        m_LightPointBuffer->Bind(3);
    }
    if (m_MaterialBuffer) {
        m_MaterialBuffer->Bind(4);
    }
    if (m_VertexBuffer) {
        m_VertexBuffer->Bind(5);
    }

    assert(m_AmbientOcclusionTemporalTexture2D);

    m_AmbientOcclusionTemporalTexture2D->Bind(0, m_SamplerClamp);

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

    if (m_ShadowCubeColorTextureCubeArray) {
        m_ShadowCubeColorTextureCubeArray->Bind(7, m_SamplerClamp);  
    }
    if (m_ShadowCubeDepthTextureCubeArray) {
        m_ShadowCubeDepthTextureCubeArray->Bind(8, m_SamplerClamp);  
    }

    m_LightingShaderProgram->Use();

    glUniform1i(0, m_EnableAmbientOcclusion);
    glUniform1i(1, m_EnableReverseZ);
    glUniform1ui(2, g_LightPoints.size());
    glUniform1f(3, 1.f / SHADOW_CSM_SIZE * m_ShadowCsmFilterRadius);
    glUniform1f(4, 1.f / SHADOW_CUBE_SIZE * m_ShadowCubeFilterRadius);

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