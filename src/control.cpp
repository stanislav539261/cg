#include <glm/glm.hpp>

#include "control.hpp"
#include "ui.hpp"

std::shared_ptr<Control> g_Control = nullptr; 

Control::Control() {
    m_CameraDirection = {};
    m_CameraPitch = 0.f;
    m_CameraYaw = 0.f;
    m_KeysPressedOnce = {};
    m_KeysPressedRepeat = {};
}

Control::~Control() {
    
}

void Control::Update(const std::vector<SDL_Event> &events) {
    constexpr float MOUSE_SENSITIVITY = 0.2f;
    
    auto pitchOffset = 0.f;
    auto yawOffset = 0.f;
    
    m_KeysPressedRepeat.insert(std::begin(m_KeysPressedOnce), std::end(m_KeysPressedOnce));
    m_KeysPressedOnce.clear();

    for (const auto &event : events) {
        if (event.type == SDL_KEYDOWN) {
            m_KeysPressedOnce.insert(event.key.keysym.sym);
        } else if (event.type == SDL_KEYUP) {
            m_KeysPressedOnce.erase(event.key.keysym.sym);
            m_KeysPressedRepeat.erase(event.key.keysym.sym);
        } else if (event.type == SDL_MOUSEMOTION) {
            if (g_Ui && !g_Ui->m_ShowMenu) {
                pitchOffset += event.motion.yrel * MOUSE_SENSITIVITY;
                yawOffset += event.motion.xrel * MOUSE_SENSITIVITY;
            }
        }
    }

    m_CameraDirection = glm::vec3(0.f);
    m_CameraPitch = glm::clamp(m_CameraPitch + pitchOffset, -89.f, 89.f);
    m_CameraYaw += yawOffset;

    for (const auto &key : m_KeysPressedOnce) {
        switch (key) {
            case SDLK_F1:
                if (g_Ui) {
                    g_Ui->m_ShowMenu = !g_Ui->m_ShowMenu;
                }
                break;
            default:
                break;
        }
    }

    for (const auto &key : m_KeysPressedRepeat) {
        switch (key) {
            case SDLK_w:
                m_CameraDirection.z += 1.f;
                break;
            case SDLK_a:
                m_CameraDirection.x -= 1.f;
                break;
            case SDLK_s:
                m_CameraDirection.z -= 1.f;
                break;
            case SDLK_d:
                m_CameraDirection.x += 1.f;
                break;
            default:
                break;
        }
    }
}