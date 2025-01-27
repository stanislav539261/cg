#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <memory>
#include <set>
#include <vector>

#include <SDL2/SDL_events.h>

class Control {
public:
    Control();
    ~Control();

    void                        Update(const std::vector<SDL_Event> &);

    std::set<unsigned int>      m_KeysPressedRepeat;
    std::set<unsigned int>      m_KeysPressedOnce;
};

extern std::shared_ptr<Control> g_Control;

#endif /* CONTROL_HPP */