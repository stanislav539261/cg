#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>

#include <glm/glm.hpp>

#include "object.hpp"

class Camera : public Object {
public:
    Camera();
    ~Camera();

    float       AspectRatio() const;
    glm::vec3   Forward() const;
    bool        IsCamera() const override { return true; }
    glm::mat4   Projection(bool) const;
    void        Translate(const glm::vec3 &);
    void        Update() override;
    glm::mat4   View() const;

    float       m_FarZ;
    float       m_FovY;
    float       m_NearZ;
    float       m_Pitch;
    glm::vec3   m_Up;
    float       m_Yaw;
};

#endif /* CAMERA_HPP */
