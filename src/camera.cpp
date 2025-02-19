#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "camera.hpp"
#include "state.hpp"

std::shared_ptr<Camera> g_Camera = nullptr;

Camera::Camera(
    float aspectRatio, 
    float fovY,
    float nearZ,
    float farZ, 
    const glm::vec3 &position,
    float pitch, 
    float yaw
) : Object() {
    m_Position = position;
    m_Up = glm::vec3(0.f, 1.f, 0.f);
    m_NearZ = nearZ;
    m_FarZ = farZ;
    m_AspectRatio = aspectRatio;
    m_Pitch = 0.f;
    m_Yaw = 0.f;
    m_FovY = fovY;
}

Camera::~Camera() {

}

glm::vec3 Camera::Forward() const {
    return glm::normalize(glm::vec3(
        cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch)),
        sin(glm::radians(m_Pitch)),
        sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch))
    ));
}

glm::mat4 Camera::Projection(bool reversedZ) const {
    auto fovY = glm::radians(m_FovY);
    auto halfFovY = fovY * 0.5f;
    auto v = glm::tan(halfFovY);
    auto h = m_AspectRatio * v;
    auto halfFovX = glm::atan(h);
    auto fovX = halfFovX * 2.f;

    return glm::perspectiveZO(
        fovY, 
        fovX, 
        reversedZ ? m_FarZ : m_NearZ, 
        reversedZ ? m_NearZ : m_FarZ
    );
}

void Camera::Translate(const glm::vec3 &direction) {
    auto forward = Forward();
    auto speed = 1000.f * g_DeltaTime;

    m_Position += speed * forward * direction.z;
    m_Position += speed * glm::normalize(glm::cross(forward, m_Up)) * direction.x;
}

glm::mat4 Camera::View() const {
    auto forward = Forward();

    return glm::lookAt(m_Position, m_Position + forward, m_Up);
}
