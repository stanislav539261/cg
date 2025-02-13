#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "camera.hpp"

class LightEnvironment {
public:
    LightEnvironment(const glm::vec3 &, const glm::vec3 &, float, float);
    ~LightEnvironment();

    std::array<glm::mat4, 5>    CascadeViewProjections(const Camera &, const std::array<float, 4> &, bool) const;
    glm::vec3                   Forward() const;

    glm::vec3                   m_AmbientColor;
    glm::vec3                   m_BaseColor;
    float                       m_Pitch;
    float                       m_Yaw;
};

class LightPoint {
public:
    LightPoint(const glm::vec3 &, const glm::vec3 &, float, bool = false);
    ~LightPoint();

    std::array<glm::mat4, 6>    ViewProjections(bool) const;

    glm::vec3                   m_BaseColor;
    glm::vec3                   m_Position;
    float                       m_Radius;
    bool                        m_CastShadows;
};

extern std::shared_ptr<LightEnvironment> g_LightEnvironment;
extern std::vector<std::shared_ptr<LightPoint>> g_LightPoints;

#endif /* LIGHT_HPP */