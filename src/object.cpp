#include <glm/ext/quaternion_trigonometric.hpp>

#include "object.hpp"

glm::vec3 Object::Forward() const {
    const auto rotationInversed = glm::inverse(Rotation());
    const auto up = rotationInversed * glm::vec3(0.f, 1.f, 0.f);
    const auto right = rotationInversed * glm::vec3(1.f, 0.f, 0.f);

    return glm::normalize(glm::cross(up, right));
}

glm::quat Object::Rotation() const {
    const auto qx = glm::angleAxis(glm::radians(m_Angles.x), glm::vec3(1.f, 0.f, 0.f));
    const auto qy = glm::angleAxis(glm::radians(m_Angles.y), glm::vec3(0.f, 1.f, 0.f));
    const auto qz = glm::angleAxis(glm::radians(m_Angles.z), glm::vec3(0.f, 0.f, 1.f));

    return glm::normalize(qx * qz * qy);
}

void Object::Translate(const glm::vec3 &velocity) {
    const auto forward = Forward();

    m_Position += forward * velocity.z;
    m_Position += glm::normalize(glm::cross(forward, glm::vec3(0.f, 1.f, 0.f))) * velocity.x;
}