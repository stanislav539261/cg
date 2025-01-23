#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <memory>

#include <SDL2/SDL_video.h>

class Window {
public:
    Window();
    ~Window();

    void            Update();

    SDL_Window *    m_Window;
};

extern std::shared_ptr<Window> g_Window;

#endif /* WINDOW_HPP */