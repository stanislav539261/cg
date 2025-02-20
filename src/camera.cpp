#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "camera.hpp"
#include "control.hpp"
#include "render.hpp"
#include "state.hpp"
#include "ui.hpp"
#include "window.hpp"

Camera::Camera() : Object() {
    m_Position = {};
    m_Up = glm::vec3(0.f, 1.f, 0.f);
    m_NearZ = 1.f;
    m_FarZ = 100000.f;
    m_Pitch = 0.f;
    m_Yaw = 0.f;
    m_FovY = 78.f;
}

Camera::~Camera() {

}

float Camera::AspectRatio() const {
    return g_Window->m_ScreenWidth / static_cast<float>(g_Window->m_ScreenHeight);
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
    auto h = AspectRatio() * v;
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

void Camera::Update() {
    m_Pitch = glm::clamp(m_Pitch + g_Control->m_PitchOffset, -89.f, 89.f);
    m_Yaw += g_Control->m_YawOffset;

    Translate(g_Control->m_Direction);

    g_Render->m_DrawableActiveCamera = this;
    g_Ui->m_ActiveCamera = this;
}

glm::mat4 Camera::View() const {
    auto forward = Forward();

    return glm::lookAt(m_Position, m_Position + forward, m_Up);
}
