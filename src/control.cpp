#include <glm/glm.hpp>

#include "control.hpp"
#include "ui.hpp"

std::shared_ptr<Control> g_Control = nullptr; 

Control::Control() {

}

Control::~Control() {
    
}

void Control::Update(const std::vector<SDL_Event> &events) {
    constexpr float MOUSE_SENSITIVITY = 0.2f;

    m_PitchOffset = 0.f;
    m_YawOffset = 0.f;
    
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
                m_PitchOffset -= event.motion.yrel * MOUSE_SENSITIVITY;
                m_YawOffset += event.motion.xrel * MOUSE_SENSITIVITY;
            }
        }
    }

    m_Direction = glm::vec3(0.f);

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
                m_Direction.z += 1.f;
                break;
            case SDLK_a:
                m_Direction.x -= 1.f;
                break;
            case SDLK_s:
                m_Direction.z -= 1.f;
                break;
            case SDLK_d:
                m_Direction.x += 1.f;
                break;
            default:
                break;
        }
    }

    // for (const auto &[handle, object] : g_Scene->Objects()) {
    //     if (object->IsCamera()) {
    //         auto camera = reinterpret_cast<Camera *>(object.get());

    //         camera->m_Pitch = glm::clamp(camera->m_Pitch + pitchOffset, -89.f, 89.f);
    //         camera->m_Yaw += yawOffset;
    //         camera->Translate(direction);
    //     }
    // }
}