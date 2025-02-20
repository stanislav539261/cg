#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>

#include "light.hpp"
#include "render.hpp"
#include "ui.hpp"

LightEnvironment::LightEnvironment() : Object() {
    m_AmbientColor = {};
    m_BaseColor = {};
}

LightEnvironment::~LightEnvironment() {
    
}

std::array<glm::mat4, 5> LightEnvironment::CascadeViewProjections(const Camera *camera, const std::array<float, 4> &levels, bool reversedZ) const {
    auto cascades = std::array<glm::mat4, 5>();

    const auto computeLightSpaceMatrix = [this, camera, reversedZ](auto near, auto far) {
        auto fovY = glm::radians(camera->m_FovY);
        auto halfFovY = fovY * 0.5f;
        auto v = glm::tan(halfFovY);
        auto h = camera->AspectRatio() * v;
        auto halfFovX = glm::atan(h);
        auto fovX = halfFovX * 2.f;

        auto projection = glm::perspectiveZO(fovY, fovX, near, far);
        auto inversedViewProjection = glm::inverse(projection * camera->View());
        auto corners = std::vector<glm::vec4>();
        
        for (auto x = 0u; x < 2; x++) {
            for (auto y = 0u; y < 2; y++) {
                for (auto z = 0u; z < 2; z++) {
                    auto pt = inversedViewProjection * glm::vec4(2.f * x - 1.f, 2.f * y - 1.f, 2.f * z - 1.f, 1.f);

                    corners.push_back(pt / pt.w);
                }
            }
        }

        auto center = glm::vec3(0.f);

        for (const auto& corner : corners) {
            center += glm::vec3(corner);
        }
        
        center /= static_cast<float>(corners.size());

        auto minX = std::numeric_limits<float>::max();
        auto maxX = std::numeric_limits<float>::lowest();
        auto minY = std::numeric_limits<float>::max();
        auto maxY = std::numeric_limits<float>::lowest();
        auto minZ = std::numeric_limits<float>::max();
        auto maxZ = std::numeric_limits<float>::lowest();
        auto view = glm::lookAt(center + Forward(), center, glm::vec3(0.f, 1.f, 0.f));

        for (const auto& corner : corners) {
            auto trf = view * corner;

            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        constexpr float zMult = 10.0f;

        if (minZ < 0.f) {
            minZ *= zMult;
        } else {
            minZ /= zMult;
        }
        if (maxZ < 0.f) {
            maxZ /= zMult;
        } else {
            maxZ *= zMult;
        }

        return glm::orthoZO(minX, maxX, minY, maxY, reversedZ ? minZ : maxZ, reversedZ ? maxZ : minZ) * view;
    };

    for (auto i = 0u; i < levels.size() + 1; ++i){
        if (i == 0){
            cascades[i] = computeLightSpaceMatrix(camera->m_NearZ, levels[i]);
        } else if (i < levels.size()){
            cascades[i] = computeLightSpaceMatrix(levels[i - 1], levels[i]);
        } else {
            cascades[i] = computeLightSpaceMatrix(levels[i - 1], camera->m_FarZ);
        }
    }

    return cascades;
}

void LightEnvironment::Update() {
    g_Render->m_DrawableLightEnvironment = this;
    g_Ui->m_LightEnvironment = this;
}

LightPoint::LightPoint() : Object() {
    m_BaseColor = {};
    m_Position = {};
    m_Radius = 0.f;
    m_CastShadows = false;
}

LightPoint::~LightPoint() {
    
}

void LightPoint::Update() {
    g_Render->m_DrawableLightPoints.push_back(this);
    g_Ui->m_LightPoints.push_back(this);
}

std::array<glm::mat4, 6> LightPoint::ViewProjections(bool reversedZ) const {
    const auto projection = glm::perspectiveZO(3.1415926535897932384626433832795f / 2.f, 1.f, 1.f, m_Radius);

    return std::array<glm::mat4, 6> {
        projection * glm::lookAtRH(
            m_Position,
            m_Position + glm::vec3(1.00, 0.00, 0.00),
            glm::vec3(0.00, -1.0, 0.00)
        ),
        projection * glm::lookAtRH(
            m_Position,
            m_Position + glm::vec3(-1.0, 0.00, 0.00),
            glm::vec3(0.00, -1.0, 0.00)
        ),
        projection * glm::lookAtRH(
            m_Position,
            m_Position + glm::vec3(0.00, 1.00, 0.00),
            glm::vec3(0.00, 0.00, 1.00)
        ),
        projection * glm::lookAtRH(
            m_Position,
            m_Position + glm::vec3(0.00, -1.0, 0.00),
            glm::vec3(0.00, 0.00, -1.0)
        ),
        projection * glm::lookAtRH(
            m_Position,
            m_Position + glm::vec3(0.00, 0.00, 1.00),
            glm::vec3(0.00, -1.0, 0.00)
        ),
        projection * glm::lookAtRH(
            m_Position,
            m_Position + glm::vec3(0.00, 0.00, -1.0),
            glm::vec3(0.00, -1.0, 0.00)
        )
    };
}