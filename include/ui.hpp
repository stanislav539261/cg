#ifndef UI_HPP
#define UI_HPP

#include <memory>
#include <vector>

#include <SDL2/SDL_events.h>

class Ui {
public:
    Ui();
    ~Ui();

    void                    Update(const std::vector<SDL_Event> &);

    bool                    m_ShowMenu;
};

extern std::shared_ptr<Ui> g_Ui;

#endif /* UI_HPP */