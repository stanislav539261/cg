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
constexpr GLuint  GRID_SIZE_X = 16;
constexpr GLuint  GRID_SIZE_Y = 8;
constexpr GLuint  GRID_SIZE_Z = 24;
constexpr size_t  MAX_LIGHT_POINTS = 1024;
constexpr GLuint  SHADOW_CSM_SIZE = 2048;
constexpr GLuint  SHADOW_CUBE_SIZE = 1024;
constexpr GLuint  TEXTURE_SIZE = 1024;

std::unique_ptr<Render> g_Render = nullptr;

static GLuint ComputeMipLevel(const glm::uvec2 &extent) {
    auto minHeight = extent.y;
    auto minWidth = extent.x;
    auto mipLevel = 0u;

    for (auto i = 0u; minHeight > 0 && minWidth > 0; i++) {
        mipLevel++;
        minHeight /= 2;
        minWidth /= 2;
    }

    return mipLevel;
}

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
    
    SDL_GL_SetSwapInterval(0);

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

            m_AmbientOcclusionFalloffFar = 2000.f;
            m_AmbientOcclusionFalloffNear = 1.f;
            m_AmbientOcclusionNumSamples = 4;
            m_AmbientOcclusionNumSlices = 4;
            m_AmbientOcclusionRadius = 4.f;
            m_DrawFlags = DrawFlags::Lighting;
            m_EnableAmbientOcclusion = true;
            m_EnableReverseZ = true;
            m_EnableVSync = false;
            m_EnableWireframeMode = false;
            m_NumFrames = 0;
            m_ShadowCsmFilterRadius = 2.f;
            m_ShadowCsmVarianceMax = 0.00008f;
            m_ShadowCubeFilterRadius = 2.f;
            m_ShadowCubeVarianceMax = 0.00008f;

            // Create buffers
            m_CameraBuffer = std::make_unique<Buffer<GpuCamera>>();
            m_ClusterBuffer = std::make_unique<Buffer<GpuCluster>>(GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z);
            m_LightCounterBuffer = std::make_unique<Buffer<std::uint32_t>>();
            m_LightGridBuffer = std::make_unique<const Buffer<GpuLightGrid>>(GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z);
            m_LightIndexBuffer = std::make_unique<const Buffer<std::uint32_t>>(GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z * MAX_LIGHT_POINTS);

            // Create framebuffers
            m_AmbientOcclusionFramebuffer = std::make_unique<const Framebuffer>();
            m_AmbientOcclusionSpartialFramebuffer = std::make_unique<const Framebuffer>();
            m_AmbientOcclusionTemporalFramebuffer = std::make_unique<const Framebuffer>();
            m_DepthFramebuffer = std::make_unique<const Framebuffer>();
            m_DownsampleDepthFramebuffer = std::make_unique<const Framebuffer>();
            m_LastDepthFramebuffer = std::make_unique<const Framebuffer>();
            m_LastLightingFramebuffer = std::make_unique<const Framebuffer>();
            m_LightingFramebuffer = std::make_unique<const Framebuffer>();
            m_ShadowCsmFramebuffer = std::make_unique<const Framebuffer>();
            m_ShadowCubeFramebuffer = std::make_unique<const Framebuffer>();

            // Create samplers
            m_SamplerBorderWhite = std::make_unique<const Sampler>();
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_BORDER_COLOR, glm::vec4(1.f));
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_REPEAT));
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_REPEAT));
            m_SamplerBorderWhite->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_REPEAT));

            m_SamplerClamp = std::make_unique<const Sampler>();
            m_SamplerClamp->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            m_SamplerClamp->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            m_SamplerClamp->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            m_SamplerClamp->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            m_SamplerClamp->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_CLAMP_TO_EDGE));
            m_SamplerClamp->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_CLAMP_TO_EDGE));
            m_SamplerClamp->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_CLAMP_TO_EDGE));

            m_SamplerWrap = std::make_unique<const Sampler>();
            m_SamplerWrap->SetParameter(GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(GL_LINEAR));
            m_SamplerWrap->SetParameter(GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST));
            m_SamplerWrap->SetParameter(GL_TEXTURE_MAX_LOD, 1000.f);
            m_SamplerWrap->SetParameter(GL_TEXTURE_MIN_LOD, 0.f);
            m_SamplerWrap->SetParameter(GL_TEXTURE_WRAP_R, static_cast<GLenum>(GL_REPEAT));
            m_SamplerWrap->SetParameter(GL_TEXTURE_WRAP_S, static_cast<GLenum>(GL_REPEAT));
            m_SamplerWrap->SetParameter(GL_TEXTURE_WRAP_T, static_cast<GLenum>(GL_REPEAT));

            // Create shader programs
            m_AmbientOcclusionShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_AmbientOcclusionShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "gtao.vert"));
            assert(m_AmbientOcclusionShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "gtao.frag"));
            
            m_AmbientOcclusionSpartialShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_AmbientOcclusionSpartialShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "gtao_spartial.vert"));
            assert(m_AmbientOcclusionSpartialShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "gtao_spartial.frag"));

            m_AmbientOcclusionTemporalShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_AmbientOcclusionTemporalShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "gtao_temporal.vert"));
            assert(m_AmbientOcclusionTemporalShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "gtao_temporal.frag"));

            m_ClusterShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_ClusterShaderProgram->Link(GL_COMPUTE_SHADER, g_ResourcePath / "shaders" / "compute_clusters.comp"));

            m_DepthShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_DepthShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "depth.vert"));

            m_DownsampleDepthShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_DownsampleDepthShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "downsample_depth.vert"));
            assert(m_DownsampleDepthShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "downsample_depth.frag"));

            m_LightCullingShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_LightCullingShaderProgram->Link(GL_COMPUTE_SHADER, g_ResourcePath / "shaders" / "light_culling.comp"));

            m_LightingShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_LightingShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "lighting.vert"));
            assert(m_LightingShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "lighting.frag"));

            m_ScreenShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_ScreenShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "screen.vert"));
            assert(m_ScreenShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "screen.frag"));

            m_ShadowCsmShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_ShadowCsmShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "shadow_csm.vert"));
            assert(m_ShadowCsmShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "shadow_csm.frag"));

            m_ShadowCubeShaderProgram = std::make_unique<const ShaderProgram>();
            assert(m_ShadowCubeShaderProgram->Link(GL_VERTEX_SHADER, g_ResourcePath / "shaders" / "shadow_cube.vert"));
            assert(m_ShadowCubeShaderProgram->Link(GL_FRAGMENT_SHADER, g_ResourcePath / "shaders" / "shadow_cube.frag"));

            // Create textures
            const auto screenExtent = glm::uvec2(g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);
            const auto screenMipLevel = ComputeMipLevel(screenExtent);

            m_AmbientOcclusionTexture2D = std::make_unique<const Texture2D>(screenExtent , 1, GL_R16F);
            m_AmbientOcclusionSpartialTexture2D = std::make_unique<const Texture2D>(screenExtent, 1, GL_R16F);
            m_AmbientOcclusionTemporalTexture2D = std::make_unique<const Texture2D>(screenExtent, 1, GL_R16F);
            m_DepthTexture2D = std::make_unique<const Texture2D>(screenExtent, screenMipLevel, GL_DEPTH_COMPONENT32F);
            m_LastAmbientOcclusionTemporalTexture2D = std::make_unique<const Texture2D>(screenExtent, 1, GL_R16F);
            m_LastDepthTexture2D = std::make_unique<const Texture2D>(screenExtent, screenMipLevel, GL_DEPTH_COMPONENT32F);
            m_LightingTexture2D = std::make_unique<const Texture2D>(screenExtent, 1, GL_RGBA16F);
            m_ShadowCsmColorTexture2DArray = std::make_unique<const Texture2DArray>(glm::uvec3(SHADOW_CSM_SIZE, SHADOW_CSM_SIZE, 5u), 1, GL_R32F);
            m_ShadowCsmDepthTexture2DArray = std::make_unique<const Texture2DArray>(glm::uvec3(SHADOW_CSM_SIZE, SHADOW_CSM_SIZE, 5u), 1, GL_DEPTH_COMPONENT32F);
            m_ShadowCubeColorTextureCubeArray = std::make_unique<const TextureCubeArray>(glm::uvec3(SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 6u), 1, GL_R32F);
            m_ShadowCubeDepthTextureCubeArray = std::make_unique<const TextureCubeArray>(glm::uvec3(SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 6u), 1, GL_DEPTH_COMPONENT32F);

            // Create texture views
            m_DepthTextureView2Ds = std::vector<std::unique_ptr<const TextureView2D>>();
            m_LastDepthTextureView2Ds = std::vector<std::unique_ptr<const TextureView2D>>();
            m_ShadowCubeColorTextureViewCubes = std::vector<std::unique_ptr<const TextureViewCube>>();
            m_ShadowCubeDepthTextureViewCubes = std::vector<std::unique_ptr<const TextureViewCube>>();

            for (auto i = 0u; i < screenMipLevel; i++) {
                m_DepthTextureView2Ds.push_back(std::make_unique<const TextureView2D>(m_DepthTexture2D.get(), i, screenMipLevel - i, 0));
                m_LastDepthTextureView2Ds.push_back(std::make_unique<const TextureView2D>(m_LastDepthTexture2D.get(), i, screenMipLevel - i, 0));
            }

            m_ShadowCubeColorTextureViewCubes.push_back(std::make_unique<const TextureViewCube>(m_ShadowCubeColorTextureCubeArray.get(), 0, 1, 0));
            m_ShadowCubeDepthTextureViewCubes.push_back(std::make_unique<const TextureViewCube>(m_ShadowCubeDepthTextureCubeArray.get(), 0, 1, 0));
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

    auto diffuseImages = std::vector<const Image *>();
    auto metalnessImages = std::vector<const Image *>();
    auto normalImages = std::vector<const Image *>();
    auto roughnessImages = std::vector<const Image *>();

    for (const auto &material : model.m_Materials) {
        if (material.m_DiffuseImage) {
            diffuseImages.push_back(material.m_DiffuseImage.get());
        }
        if (material.m_MetalnessImage) {
            metalnessImages.push_back(material.m_MetalnessImage.get());
        }
        if (material.m_NormalImage) {
            normalImages.push_back(material.m_NormalImage.get());
        }
        if (material.m_RoughnessImage) {
            roughnessImages.push_back(material.m_RoughnessImage.get());
        }
    }

    // Load textures
    // Use texture arrays because my GPU doesn't support bindless textures
    auto diffuseTexture2DArray = std::unique_ptr<const Texture2DArray>();
    auto metalnessTexture2DArray = std::unique_ptr<const Texture2DArray>();
    auto normalTexture2DArray = std::unique_ptr<const Texture2DArray>();
    auto roughnessTexture2DArray = std::unique_ptr<const Texture2DArray>();

    const auto extent = glm::uvec3(glm::uvec2(TEXTURE_SIZE), diffuseImages.size());
    const auto mipLevel = ComputeMipLevel(extent);

    if (diffuseImages.size() > 0) {
        diffuseTexture2DArray = std::make_unique<const Texture2DArray>(extent, mipLevel, GL_RGB8);
    }
    if (metalnessImages.size() > 0) {
        metalnessTexture2DArray = std::make_unique<const Texture2DArray>(extent, mipLevel, GL_R8);
    }
    if (normalImages.size() > 0) {
        normalTexture2DArray = std::make_unique<const Texture2DArray>(extent, mipLevel, GL_RGB8);
    }
    if (roughnessImages.size() > 0) {
        roughnessTexture2DArray = std::make_unique<const Texture2DArray>(extent, mipLevel, GL_R8);
    }

    for (auto i = 0u; i < diffuseImages.size(); i++) {
        diffuseTexture2DArray->Upload(diffuseImages[i], glm::uvec3(0, 0, i), 0);
    }
    for (auto i = 0u; i < metalnessImages.size(); i++) {
        metalnessTexture2DArray->Upload(metalnessImages[i], glm::uvec3(0, 0, i), 0);
    }
    for (auto i = 0u; i < normalImages.size(); i++) {
        normalTexture2DArray->Upload(normalImages[i], glm::uvec3(0, 0, i), 0);
    }
    for (auto i = 0u; i < roughnessImages.size(); i++) {
        roughnessTexture2DArray->Upload(roughnessImages[i], glm::uvec3(0, 0, i), 0);
    }

    diffuseTexture2DArray->GenerateMipMaps();
    metalnessTexture2DArray->GenerateMipMaps();
    normalTexture2DArray->GenerateMipMaps();
    roughnessTexture2DArray->GenerateMipMaps();

    // Load buffers
    auto materialBuffer = std::make_unique<const Buffer<GpuMaterial>>(model.m_Materials.size());

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

    materialBuffer->Upload(materials, 0);

    auto drawIndirectBuffer = std::make_unique<const DrawIndirectBuffer>(model.m_Meshes.size());
    auto indexBuffer = std::make_unique<const Buffer<GpuIndex>>(model.NumIndices());
    auto lightEnvironmentBuffer = std::make_unique<const Buffer<GpuLightEnvironment>>();
    auto lightPointBuffer = std::make_unique<const Buffer<GpuLightPoint>>(MAX_LIGHT_POINTS);
    auto vertexBuffer = std::make_unique<const Buffer<GpuVertex>>(model.NumVertices());

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

        auto indices = std::vector<std::uint32_t>();

        indices.reserve(mesh.m_Indices.size());

        for (const auto &index : mesh.m_Indices) {
            indices.push_back(vertexOffset + index);
        }

        auto &vertices = mesh.m_Vertices;

        drawIndirectBuffer->Upload(drawIndirectCommand, meshes.size());
        indexBuffer->Upload(indices, indexOffset);
        vertexBuffer->Upload(vertices, vertexOffset);

        indexOffset += indices.size();
        vertexOffset += vertices.size();

        meshes.push_back(std::make_tuple(indexOffset, indices.size()));
    }

    m_DiffuseTexture2DArray = std::move(diffuseTexture2DArray);
    m_DrawIndirectBuffer = std::move(drawIndirectBuffer);
    m_IndexBuffer = std::move(indexBuffer);
    m_LightEnvironmentBuffer = std::move(lightEnvironmentBuffer);
    m_LightPointBuffer = std::move(lightPointBuffer);
    m_MaterialBuffer = std::move(materialBuffer);
    m_Meshes = std::move(meshes);
    m_MetalnessTexture2DArray = std::move(metalnessTexture2DArray);
    m_NormalTexture2DArray = std::move(normalTexture2DArray);
    m_RoughnessTexture2DArray = std::move(roughnessTexture2DArray);
    m_VertexBuffer = std::move(vertexBuffer);
}

void Render::Update() {
    if (!m_Context) {
        return;
    }

    if (m_EnableVSync) {
        if (SDL_GL_GetSwapInterval() != 1) {
            SDL_GL_SetSwapInterval(1);
        }
    } else {
        if (SDL_GL_GetSwapInterval() != 0) {
            SDL_GL_SetSwapInterval(0);
        }
    }

    auto cameraUploadData = std::vector<GpuCamera>();
    auto lightEnvironmentUploadData = std::vector<GpuLightEnvironment>();
    auto lightPointUploadData = std::vector<GpuLightPoint>();
    auto numLightPointShadows = 0u;

    if (g_Camera) {
        static glm::mat4 lastView = glm::identity<glm::mat4>();

        const auto projection = g_Camera->Projection(m_EnableReverseZ);
        const auto projectionNonReversed = m_EnableReverseZ ? g_Camera->Projection(false) : projection;
        const auto fovY = glm::radians(g_Camera->m_FovY);
        const auto halfFovY = fovY * 0.5f;
        const auto v = glm::tan(halfFovY);
        const auto h = g_Camera->m_AspectRatio * v;
        const auto halfFovX = glm::atan(h);
        const auto fovX = halfFovX * 2.f;
        const auto view = g_Camera->View();
        const auto normTileDim = glm::vec2(1.f / static_cast<float>(GRID_SIZE_X), 1.f / static_cast<float>(GRID_SIZE_Y));
        const auto tileSizeInv = glm::vec2(1.f / (g_Window->m_ScreenWidth * normTileDim.x), 1.f / (g_Window->m_ScreenHeight * normTileDim.y));
        const auto sliceBiasFactor = -((static_cast<float>(GRID_SIZE_Z) * std::log2(g_Camera->m_NearZ)) / std::log2(g_Camera->m_FarZ / g_Camera->m_NearZ));
        const auto sliceScalingFactor = static_cast<float>(GRID_SIZE_Z) / std::log2(g_Camera->m_FarZ / g_Camera->m_NearZ);

        cameraUploadData.push_back(GpuCamera {
            .m_LastView = lastView,
            .m_Projection = projection,
            .m_ProjectionInversed = glm::inverse(projection),
            .m_ProjectionNonReversed = projectionNonReversed,
            .m_ProjectionNonReversedInversed = glm::inverse(projectionNonReversed),
            .m_View = view,
            .m_Position = g_Camera->m_Position,
            .m_NormTileDim = normTileDim,
            .m_TileSizeInv = tileSizeInv,
            .m_FarZ = g_Camera->m_FarZ,
            .m_NearZ = g_Camera->m_NearZ,
            .m_FovX = fovX,
            .m_FovY = fovY,
            .m_SliceBiasFactor = sliceBiasFactor,
            .m_SliceScalingFactor = sliceScalingFactor,
        });

        lastView = std::move(view);
    }

    if (g_LightEnvironment) {
        auto cascadeLevels = std::array<float, 4> {
            g_Camera->m_FarZ * 1.f / 80.f,
            g_Camera->m_FarZ * 1.f / 40.f,
            g_Camera->m_FarZ * 1.f / 20.f,
            g_Camera->m_FarZ * 1.f / 10.f,
        };

        lightEnvironmentUploadData.push_back(GpuLightEnvironment {
            .m_CascadeViewProjections = g_LightEnvironment->CascadeViewProjections(*g_Camera, cascadeLevels, m_EnableReverseZ),
            .m_CascadePlaneDistances = cascadeLevels,
            .m_AmbientColor = g_LightEnvironment->m_AmbientColor,
            .m_BaseColor = g_LightEnvironment->m_BaseColor,
            .m_Direction = g_LightEnvironment->Forward(),
        });
    }

    for (auto i = 0u; i < std::min(g_LightPoints.size(), MAX_LIGHT_POINTS); i++) {
        const auto &lightPoint = g_LightPoints[i];

        lightPointUploadData.push_back(GpuLightPoint {
            .m_ViewProjections = lightPoint->ViewProjections(m_EnableReverseZ),
            .m_Position = lightPoint->m_Position,
            .m_Radius = lightPoint->m_Radius,
            .m_BaseColor = lightPoint->m_BaseColor,
            .m_ShadowIndex = lightPoint->m_CastShadows ? static_cast<std::int32_t>(numLightPointShadows) : -1,
        });

        if (lightPoint->m_CastShadows) {
            numLightPointShadows++;
        }
    }
    
    assert(m_CameraBuffer);
    assert(m_LightCounterBuffer);
    assert(m_LightEnvironmentBuffer);
    assert(m_LightPointBuffer);

    m_CameraBuffer->Upload(cameraUploadData, 0);
    m_LightCounterBuffer->Upload(0, 0);
    m_LightEnvironmentBuffer->Upload(lightEnvironmentUploadData, 0);
    m_LightPointBuffer->Upload(lightPointUploadData, 0);

    // Recreate shadow cubes
    assert(m_ShadowCubeColorTextureCubeArray);

    if (m_ShadowCubeColorTextureCubeArray->m_Extent.z < numLightPointShadows * 6) {
        const auto extent = glm::uvec3(glm::uvec2(SHADOW_CUBE_SIZE), std::max(numLightPointShadows * 6lu, 6lu));

        m_ShadowCubeColorTextureCubeArray = std::make_unique<const TextureCubeArray>(extent, 1, GL_R16);
        m_ShadowCubeColorTextureViewCubes.clear();
    
        while (m_ShadowCubeColorTextureViewCubes.size() < std::max(numLightPointShadows, 1u)) {
            m_ShadowCubeColorTextureViewCubes.push_back(
                std::make_unique<const TextureViewCube>(
                    m_ShadowCubeColorTextureCubeArray.get(), 
                    0, 
                    1, 
                    m_ShadowCubeColorTextureViewCubes.size() * 6
                )
            );
        }
    }

    assert(m_ShadowCubeDepthTextureCubeArray);

    if (m_ShadowCubeDepthTextureCubeArray->m_Extent.z < numLightPointShadows * 6) {
        const auto extent = glm::uvec3(glm::uvec2(SHADOW_CUBE_SIZE), std::max(numLightPointShadows * 6lu, 6lu));

        m_ShadowCubeDepthTextureCubeArray = std::make_unique<TextureCubeArray>(extent, 1, GL_DEPTH_COMPONENT16);
        m_ShadowCubeDepthTextureViewCubes.clear();

        while (m_ShadowCubeDepthTextureViewCubes.size() < std::max(numLightPointShadows, 1u)) {
            m_ShadowCubeDepthTextureViewCubes.push_back(
                std::make_unique<const TextureViewCube>(
                    m_ShadowCubeDepthTextureCubeArray.get(),
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
    ClusterPass();
    LightCullingPass();
    LightingPass();
    ScreenPass();

    m_NumFrames++;
}

void Render::ShadowCsmPass() {;
    assert(m_ShadowCsmFramebuffer);
    assert(m_ShadowCsmShaderProgram);

    m_ShadowCsmFramebuffer->Bind();
    m_ShadowCsmShaderProgram->Use();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDepthFunc(m_EnableReverseZ ? GL_GEQUAL : GL_LEQUAL);
    glDepthMask(true);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_ShadowCsmColorTexture2DArray->m_Extent.x, m_ShadowCsmColorTexture2DArray->m_Extent.y);
    glViewport(0, 0, m_ShadowCsmColorTexture2DArray->m_Extent.x, m_ShadowCsmColorTexture2DArray->m_Extent.y);

    m_ShadowCsmFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_ShadowCsmColorTexture2DArray.get());
    m_ShadowCsmFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_ShadowCsmDepthTexture2DArray.get());
    m_ShadowCsmFramebuffer->ClearColor(0, m_EnableReverseZ ? glm::vec4(0.f) : glm::vec4(1.f));
    m_ShadowCsmFramebuffer->ClearDepth(0, m_EnableReverseZ ? 0.f : 1.f);

    assert(m_DrawIndirectBuffer);
    assert(m_IndexBuffer);
    assert(m_LightEnvironmentBuffer);
    assert(m_VertexBuffer);

    m_DrawIndirectBuffer->BindIndirect();
    m_IndexBuffer->BindStorage(0);
    m_LightEnvironmentBuffer->BindStorage(1);
    m_VertexBuffer->BindStorage(2);

    for (auto i = 0u; i < m_ShadowCsmColorTexture2DArray->m_Extent.z; i++) {
        glUniform1ui(0, i);

        glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
    }
}

void Render::ShadowCubePass() {
    assert(m_ShadowCubeFramebuffer);
    assert(m_ShadowCubeShaderProgram);

    m_ShadowCubeFramebuffer->Bind();
    m_ShadowCubeShaderProgram->Use();

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_ShadowCubeColorTextureCubeArray->m_Extent.x, m_ShadowCubeColorTextureCubeArray->m_Extent.y);
    glViewport(0, 0, m_ShadowCubeColorTextureCubeArray->m_Extent.x, m_ShadowCubeColorTextureCubeArray->m_Extent.y);

    assert(m_DrawIndirectBuffer);
    assert(m_IndexBuffer);
    assert(m_LightPointBuffer);
    assert(m_VertexBuffer);

    m_DrawIndirectBuffer->BindIndirect();
    m_IndexBuffer->BindStorage(0);
    m_LightPointBuffer->BindStorage(1);
    m_VertexBuffer->BindStorage(2);

    auto numLightPointShadows = 0u;

    for (auto i = 0u; i < std::min(g_LightPoints.size(), MAX_LIGHT_POINTS); i++) {
        const auto &lightPoint = g_LightPoints[i];

        if (!lightPoint->m_CastShadows) {
            continue;
        }

        m_ShadowCubeFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_ShadowCubeColorTextureViewCubes.at(numLightPointShadows).get());
        m_ShadowCubeFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_ShadowCubeDepthTextureViewCubes.at(numLightPointShadows).get());
        m_ShadowCubeFramebuffer->ClearColor(0, glm::vec4(1.f));
        m_ShadowCubeFramebuffer->ClearDepth(0, 1.f);

        glUniform1ui(1, i);

        for (auto j = 0u; j < 6; j++) {
            glUniform1ui(0, j);

            glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
        }

        numLightPointShadows++;
    }
}

void Render::DepthPass() {
    assert(m_DepthFramebuffer);
    assert(m_DepthShaderProgram);

    m_DepthFramebuffer->Bind();
    m_DepthShaderProgram->Use();

    assert(m_DepthTexture2D);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDepthFunc(m_EnableReverseZ ? GL_GEQUAL : GL_LEQUAL);
    glDepthMask(true);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, m_EnableWireframeMode ? GL_LINE : GL_FILL);
    glScissor(0, 0, m_DepthTexture2D->m_Extent.x, m_DepthTexture2D->m_Extent.y);
    glViewport(0, 0, m_DepthTexture2D->m_Extent.x, m_DepthTexture2D->m_Extent.y);

    m_DepthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_DepthTexture2D.get());
    m_DepthFramebuffer->ClearDepth(0, m_EnableReverseZ ? 0.f : 1.f);

    assert(m_CameraBuffer);
    assert(m_DrawIndirectBuffer);
    assert(m_IndexBuffer);
    assert(m_VertexBuffer);

    m_DrawIndirectBuffer->BindIndirect();
    m_CameraBuffer->BindStorage(0);
    m_IndexBuffer->BindStorage(1);
    m_VertexBuffer->BindStorage(2);

    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
}

void Render::DownsampleDepthPass() {
    assert(m_DownsampleDepthFramebuffer);
    assert(m_DownsampleDepthShaderProgram);

    m_DownsampleDepthFramebuffer->Bind();
    m_DownsampleDepthShaderProgram->Use();
    
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(true);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUniform1i(0, m_EnableReverseZ);

    assert(m_DepthTexture2D);

    auto height = m_DepthTexture2D->m_Extent.y;
    auto width = m_DepthTexture2D->m_Extent.x;

    for (auto i = 0u; i < m_DepthTexture2D->m_MipLevel - 1; i++) {
        width /= 2;
        height /= 2;

        glScissor(0, 0, width, height);
        glViewport(0, 0, width, height);

        m_DownsampleDepthFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_DepthTextureView2Ds.at(i + 1).get());

        m_DepthTextureView2Ds.at(i + 0)->Bind(0, m_SamplerClamp.get());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Render::AmbientOcclusionPass() {
    assert(m_AmbientOcclusionFramebuffer);
    assert(m_AmbientOcclusionShaderProgram);

    m_AmbientOcclusionFramebuffer->Bind();
    m_AmbientOcclusionShaderProgram->Use();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_AmbientOcclusionTexture2D->m_Extent.x, m_AmbientOcclusionTexture2D->m_Extent.y);
    glViewport(0, 0, m_AmbientOcclusionTexture2D->m_Extent.x, m_AmbientOcclusionTexture2D->m_Extent.y);

    m_AmbientOcclusionFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_AmbientOcclusionTexture2D.get());

    assert(m_CameraBuffer);

    m_CameraBuffer->BindStorage(0);

    m_DepthTextureView2Ds.at(0)->Bind(0, m_SamplerClamp.get());

    constexpr auto OFFSETS = std::array<float, 4> { 0.0f, 0.5f, 0.25f, 0.75f };
    constexpr auto ROTATIONS = std::array<float, 6> { 60.f, 300.f, 180.f, 240.f, 120.f, 0.f };

    glUniform1f(0, m_AmbientOcclusionFalloffFar);
    glUniform1f(1, m_AmbientOcclusionFalloffNear);
    glUniform1ui(2, std::max(m_AmbientOcclusionNumSamples, 1));
    glUniform1ui(3, std::max(m_AmbientOcclusionNumSlices, 1));
    glUniform1f(4, OFFSETS[m_NumFrames / 6 % OFFSETS.size()]);
    glUniform1f(5, m_AmbientOcclusionRadius);
    glUniform1f(6, ROTATIONS[m_NumFrames % 6] / 360.f);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionSpartialPass() {
    assert(m_AmbientOcclusionSpartialFramebuffer);
    assert(m_AmbientOcclusionSpartialShaderProgram);

    m_AmbientOcclusionSpartialFramebuffer->Bind();
    m_AmbientOcclusionSpartialShaderProgram->Use();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_AmbientOcclusionSpartialTexture2D->m_Extent.x, m_AmbientOcclusionSpartialTexture2D->m_Extent.y);
    glViewport(0, 0, m_AmbientOcclusionSpartialTexture2D->m_Extent.x, m_AmbientOcclusionSpartialTexture2D->m_Extent.y);

    m_AmbientOcclusionSpartialFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_AmbientOcclusionSpartialTexture2D.get());

    assert(m_CameraBuffer);

    m_CameraBuffer->BindStorage(0);

    assert(m_AmbientOcclusionTexture2D);

    m_AmbientOcclusionTexture2D->Bind(0, m_SamplerClamp.get());
    m_DepthTextureView2Ds.at(0)->Bind(1, m_SamplerClamp.get());

    glUniform1i(0, m_EnableReverseZ);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::AmbientOcclusionTemporalPass() {
    assert(m_AmbientOcclusionTemporalFramebuffer);
    assert(m_AmbientOcclusionTemporalShaderProgram);

    m_AmbientOcclusionTemporalFramebuffer->Bind();
    m_AmbientOcclusionTemporalShaderProgram->Use();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, m_AmbientOcclusionTemporalTexture2D->m_Extent.x, m_AmbientOcclusionTemporalTexture2D->m_Extent.y);
    glViewport(0, 0, m_AmbientOcclusionTemporalTexture2D->m_Extent.x, m_AmbientOcclusionTemporalTexture2D->m_Extent.y);
    
    m_AmbientOcclusionTemporalFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_AmbientOcclusionTemporalTexture2D.get());

    assert(m_CameraBuffer);

    m_CameraBuffer->BindStorage(0);

    assert(m_AmbientOcclusionSpartialTexture2D);
    assert(m_LastAmbientOcclusionTemporalTexture2D);

    m_AmbientOcclusionSpartialTexture2D->Bind(0, m_SamplerClamp.get());
    m_DepthTextureView2Ds.at(1)->Bind(1, m_SamplerClamp.get());
    m_LastAmbientOcclusionTemporalTexture2D->Bind(2, m_SamplerClamp.get());
    m_LastDepthTextureView2Ds.at(1)->Bind(3, m_SamplerClamp.get());
   
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Render::ClusterPass() {
    assert(m_ClusterShaderProgram);
    
    m_ClusterShaderProgram->Use();

    assert(m_CameraBuffer);
    assert(m_ClusterBuffer);

    m_CameraBuffer->BindStorage(0);
    m_ClusterBuffer->BindStorage(1);

    glDispatchCompute(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Render::LightCullingPass() {
    assert(m_LightCullingShaderProgram);
    
    m_LightCullingShaderProgram->Use();

    assert(m_CameraBuffer);
    assert(m_ClusterBuffer);
    assert(m_LightCounterBuffer);
    assert(m_LightGridBuffer);
    assert(m_LightIndexBuffer);
    assert(m_LightPointBuffer);

    m_CameraBuffer->BindStorage(0);
    m_ClusterBuffer->BindStorage(1);
    m_LightCounterBuffer->BindStorage(2);
    m_LightGridBuffer->BindStorage(3);
    m_LightIndexBuffer->BindStorage(4);
    m_LightPointBuffer->BindStorage(5);

    glUniform1ui(0, g_LightPoints.size());

    glDispatchCompute(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Render::LightingPass() {
    assert(m_LightingFramebuffer);
    assert(m_LightingShaderProgram);

    m_LightingFramebuffer->Bind();
    m_LightingShaderProgram->Use();

    assert(m_LightingTexture2D);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glDepthFunc(GL_EQUAL);
    glDepthMask(false);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, m_EnableWireframeMode ? GL_LINE : GL_FILL);
    glScissor(0, 0, m_LightingTexture2D->m_Extent.x, m_LightingTexture2D->m_Extent.y);
    glViewport(0, 0, m_LightingTexture2D->m_Extent.x, m_LightingTexture2D->m_Extent.y);

    assert(m_DepthTexture2D);

    m_LightingFramebuffer->SetAttachment(GL_COLOR_ATTACHMENT0, m_LightingTexture2D.get());
    m_LightingFramebuffer->SetAttachment(GL_DEPTH_ATTACHMENT, m_DepthTexture2D.get());
    m_LightingFramebuffer->ClearColor(0, glm::vec4(glm::vec3(0.f), 1.f));

    assert(m_CameraBuffer);
    assert(m_IndexBuffer);
    assert(m_LightEnvironmentBuffer);
    assert(m_LightGridBuffer);
    assert(m_LightIndexBuffer);
    assert(m_LightPointBuffer);
    assert(m_MaterialBuffer);
    assert(m_VertexBuffer);

    m_DrawIndirectBuffer->BindIndirect();
    m_CameraBuffer->BindStorage(0);
    m_IndexBuffer->BindStorage(1);
    m_LightEnvironmentBuffer->BindStorage(2);
    m_LightGridBuffer->BindStorage(3);
    m_LightIndexBuffer->BindStorage(4);
    m_LightPointBuffer->BindStorage(5);
    m_MaterialBuffer->BindStorage(6);
    m_VertexBuffer->BindStorage(7);

    assert(m_AmbientOcclusionTemporalTexture2D);
    assert(m_DiffuseTexture2DArray);
    assert(m_MetalnessTexture2DArray);
    assert(m_NormalTexture2DArray);
    assert(m_RoughnessTexture2DArray);
    assert(m_ShadowCsmColorTexture2DArray);
    assert(m_ShadowCsmDepthTexture2DArray);
    assert(m_ShadowCubeColorTextureCubeArray);
    assert(m_ShadowCubeDepthTextureCubeArray);

    m_AmbientOcclusionTemporalTexture2D->Bind(0, m_SamplerClamp.get());
    m_DiffuseTexture2DArray->Bind(1, m_SamplerWrap.get());
    m_MetalnessTexture2DArray->Bind(2, m_SamplerWrap.get());
    m_NormalTexture2DArray->Bind(3, m_SamplerWrap.get());
    m_RoughnessTexture2DArray->Bind(4, m_SamplerWrap.get());
    m_ShadowCsmColorTexture2DArray->Bind(5, m_SamplerBorderWhite.get());
    m_ShadowCsmDepthTexture2DArray->Bind(6, m_SamplerBorderWhite.get());
    m_ShadowCubeColorTextureCubeArray->Bind(7, m_SamplerClamp.get());  
    m_ShadowCubeDepthTextureCubeArray->Bind(8, m_SamplerClamp.get());  

    glUniform1i(0, m_EnableAmbientOcclusion);
    glUniform1i(1, m_EnableReverseZ);
    glUniform1ui(2, g_LightPoints.size());
    glUniform1f(3, 1.f / SHADOW_CSM_SIZE * m_ShadowCsmFilterRadius);
    glUniform1f(4, m_ShadowCsmVarianceMax);
    glUniform1f(5, 1.f / SHADOW_CUBE_SIZE * m_ShadowCubeFilterRadius);
    glUniform1f(6, m_ShadowCubeVarianceMax);

    glMultiDrawArraysIndirect(GL_TRIANGLES, 0, m_Meshes.size(), sizeof(DrawIndirectCommand));
}

void Render::ScreenPass() {
    assert(m_ScreenShaderProgram);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glScissor(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);
    glViewport(0, 0, g_Window->m_ScreenWidth, g_Window->m_ScreenHeight);

    DefaultFramebuffer::Bind();
    m_ScreenShaderProgram->Use();

    assert(m_AmbientOcclusionTemporalTexture2D);
    assert(m_LightingTexture2D);

    switch (m_DrawFlags) {
        case DrawFlags::AmbientOcclusion:
            m_AmbientOcclusionTemporalTexture2D->Bind(0, m_SamplerClamp.get());
            break;
        case DrawFlags::Lighting:
            m_LightingTexture2D->Bind(0, m_SamplerClamp.get());
            break;
        default:
            break;
    }

    glUniform1ui(0, static_cast<std::uint32_t>(m_DrawFlags));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}