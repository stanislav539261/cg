#include <cstring>
#include <iostream>

#include <SDL2/SDL.h>

#include "camera.hpp"
#include "control.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "state.hpp"

int main(int argc, char **argv) {
    auto filename = "scenes/sponza.obj";

    auto i = 0u;

    for (; i < argc; i++) {
        if (std::strcmp(argv[i], "--scene") == 0) {
            filename = argv[++i];
        } else if (std::strcmp(argv[i], "--height") == 0) {
            g_ScreenHeight = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--width") == 0) {
            g_ScreenWidth = std::atoi(argv[++i]);
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized. " << SDL_GetError() << std::endl;
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    auto window = SDL_CreateWindow(
        "cg", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        g_ScreenWidth, 
        g_ScreenHeight, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cout << "Window could not be created. " << SDL_GetError() << std::endl;
        return 0;
    }

    auto render = Render(window);
    auto aspectRatio = g_ScreenWidth / static_cast<float>(g_ScreenHeight);

    g_Control = std::shared_ptr<Control>(new Control());
    g_MainCamera = std::shared_ptr<Camera>(new Camera(aspectRatio, 78.f, 1.f, 100000.f, glm::vec3(-200.f, 200.f, 0.f)));

    render.LoadScene(Scene(g_ResourcePath / std::filesystem::path(filename)));

    auto clock = std::chrono::system_clock::now();
    auto events = std::vector<SDL_Event>();
    auto quit = false;

    while (!quit) {
        g_PreviousTime = g_CurrentTime;
        g_CurrentTime = static_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - clock).count();
        g_DeltaTime = g_CurrentTime - g_PreviousTime;

        events.clear();

        auto event = SDL_Event {};
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
            }

            events.push_back(event);
        }

        SDL_SetRelativeMouseMode(SDL_TRUE);

        g_Control->Update(events);

        render.Update();

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    return 0;
}
