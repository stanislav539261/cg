#include <iostream>

#include "state.hpp"
#include "window.hpp"

std::shared_ptr<Window> g_Window = nullptr;

Window::Window() {
    m_Window = SDL_CreateWindow(
        "cg", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        g_ScreenWidth, 
        g_ScreenHeight, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!m_Window) {
        std::cout << "Window could not be created. " << SDL_GetError() << std::endl;
    }
}

Window::~Window() {
    SDL_DestroyWindow(m_Window);
}

void Window::Update() {
    SDL_GL_SwapWindow(g_Window->m_Window);
}