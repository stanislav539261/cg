#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <memory>
#include <set>
#include <vector>

#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>

class Control {
public:
    Control();
    ~Control();

    void                        Update(const std::vector<SDL_Event> &);

    glm::vec3                   m_CameraDirection;
    float                       m_CameraPitch;
    float                       m_CameraYaw;

private:
    std::set<unsigned int>      m_KeysPressedRepeat;
    std::set<unsigned int>      m_KeysPressedOnce;
};

extern std::shared_ptr<Control> g_Control;

#endif /* CONTROL_HPP */