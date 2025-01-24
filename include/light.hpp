#include <array>

#include <glm/glm.hpp>

#include "camera.hpp"

class LightEnvironment {
public:
    LightEnvironment(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);
    ~LightEnvironment();

    std::array<glm::mat4, 5>    CascadeViewProjections(const Camera &, const std::array<float, 4> &, bool);

    glm::vec3                   m_AmbientColor;
    glm::vec3                   m_BaseColor;
    glm::vec3                   m_Direction;
};

extern std::shared_ptr<LightEnvironment> g_LightEnvironment;