#ifndef UI_HPP
#define UI_HPP

#include "menu.hpp"
#include <memory>
#include <vector>

#include <SDL2/SDL_events.h>

class Ui {
public:
    Ui();
    ~Ui();

    void                    Update(const std::vector<SDL_Event> &);

    std::shared_ptr<Menu>   m_Menu;
};

extern std::shared_ptr<Ui> g_Ui;

#endif /* UI_HPP */