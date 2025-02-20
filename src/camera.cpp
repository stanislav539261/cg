#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "camera.hpp"
#include "control.hpp"
#include "render.hpp"
#include "state.hpp"
#include "ui.hpp"
#include "window.hpp"

Camera::Camera() : Object() {
    m_Position = {};
    m_NearZ = 1.f;
    m_FarZ = 100000.f;
    m_FovY = 78.f;
}

Camera::~Camera() {

}

float Camera::AspectRatio() const {
    return g_Window->m_ScreenWidth / static_cast<float>(g_Window->m_ScreenHeight);
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

void Camera::Update() {
    m_Angles = glm::vec3(g_Control->m_CameraPitch, g_Control->m_CameraYaw, 0.f);

    Translate(g_Control->m_CameraDirection * 1000.f * g_DeltaTime);

    g_Render->m_DrawableActiveCamera = this;
    g_Ui->m_ActiveCamera = this;
}

glm::mat4 Camera::View() const {
    return glm::mat4_cast(Rotation()) * glm::translate(glm::mat4(1.f), -m_Position);
}
