#include <glm/glm.hpp>

#include "camera.hpp"
#include "control.hpp"

std::shared_ptr<Control> g_Control = nullptr; 

Control::Control() {

}

Control::~Control() {
    
}

void Control::Update(const std::vector<SDL_Event> &events) {
    constexpr float MOUSE_SENSITIVITY = 0.2f;

    auto pitchOffset = 0.f;
    auto yawOffset = 0.f;

    for (const auto &event : events) {
        if (event.type == SDL_MOUSEMOTION) {
            pitchOffset -= event.motion.yrel * MOUSE_SENSITIVITY;
            yawOffset += event.motion.xrel * MOUSE_SENSITIVITY;
        }
    }

    g_MainCamera->m_Pitch += pitchOffset;
    g_MainCamera->m_Yaw += yawOffset;
}