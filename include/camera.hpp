#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

#include "object.hpp"

class Camera : public Object {
public:
    Camera();
    ~Camera();

    float       AspectRatio() const;
    bool        IsCamera() const override { return true; }
    bool        IsLightEnvironment() const override { return false; }
    bool        IsLightPoint() const override { return false; }
    glm::mat4   Projection(bool) const;
    void        Update() override;
    glm::mat4   View() const;

    float       m_FarZ;
    float       m_FovY;
    float       m_NearZ;
};

#endif /* CAMERA_HPP */
