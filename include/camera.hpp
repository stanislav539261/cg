#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <memory>

#include <glm/glm.hpp>

#include "object.hpp"

class Camera : public Object {
public:
    Camera(float, float, float, float, const glm::vec3 & = glm::vec3(0.f), float = 0.f, float = 0.f);
    ~Camera();

    glm::vec3   Forward() const;
    glm::mat4   Projection(bool) const;
    void        Translate(const glm::vec3 &);
    glm::mat4   View() const;

    float       m_AspectRatio;
    float       m_FarZ;
    float       m_FovY;
    float       m_NearZ;
    float       m_Pitch;
    glm::vec3   m_Up;
    float       m_Yaw;
};

extern std::shared_ptr<Camera> g_Camera;

#endif /* CAMERA_HPP */
