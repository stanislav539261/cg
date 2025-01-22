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
        if (event.type == SDL_KEYDOWN) {
            m_KeysPressed.insert(event.key.keysym.sym);
        } else if (event.type == SDL_KEYUP) {
            m_KeysPressed.erase(event.key.keysym.sym);
        } else if (event.type == SDL_MOUSEMOTION) {
            pitchOffset -= event.motion.yrel * MOUSE_SENSITIVITY;
            yawOffset += event.motion.xrel * MOUSE_SENSITIVITY;
        }
    }

    auto direction = glm::vec3(0.f);

    for (const auto &key : m_KeysPressed) {
        switch (key) {
            case SDLK_w:
                direction.z += 1.f;
                break;
            case SDLK_a:
                direction.x -= 1.f;
                break;
            case SDLK_s:
                direction.z -= 1.f;
                break;
            case SDLK_d:
                direction.x += 1.f;
                break;
            default:
                break;
        }
    }

    g_MainCamera->m_Pitch += pitchOffset;
    g_MainCamera->m_Yaw += yawOffset;
    g_MainCamera->Translate(direction);
}