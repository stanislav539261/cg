#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <memory>

#include <SDL2/SDL_video.h>

class Window {
public:
    Window(unsigned int, unsigned int);
    ~Window();

    void            Update();

    unsigned int    m_ScreenHeight;
    unsigned int    m_ScreenWidth;
    SDL_Window *    m_Window;
};

extern std::shared_ptr<Window> g_Window;

#endif /* WINDOW_HPP */