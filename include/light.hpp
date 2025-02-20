#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <array>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "object.hpp"

class LightEnvironment : public Object {
public:
    LightEnvironment();
    ~LightEnvironment();

    std::array<glm::mat4, 5>    CascadeViewProjections(const Camera *, const std::array<float, 4> &, bool) const;
    glm::vec3                   Forward() const;
    bool                        IsCamera() const override { return false; }
    bool                        IsLightEnvironment() const override { return true; }
    bool                        IsLightPoint() const override { return false; }
    void                        Update() override;

    glm::vec3                   m_AmbientColor;
    glm::vec3                   m_BaseColor;
    float                       m_Pitch;
    float                       m_Yaw;
};

class LightPoint : public Object {
public:
    LightPoint();
    ~LightPoint();

    bool                        IsCamera() const override { return false; }
    bool                        IsLightEnvironment() const override { return false; }
    bool                        IsLightPoint() const override { return true; }
    std::array<glm::mat4, 6>    ViewProjections(bool) const;
    void                        Update() override;

    glm::vec3                   m_BaseColor;
    float                       m_Radius;
    bool                        m_CastShadows;
};

#endif /* LIGHT_HPP */