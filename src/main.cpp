#include <cstring>
#include <iostream>

#include <SDL2/SDL.h>

#include "camera.hpp"
#include "control.hpp"
#include "light.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "state.hpp"
#include "ui.hpp"
#include "window.hpp"

int main(int argc, char **argv) {
    auto filename = "scenes/sponza.obj";

    auto i = 0u;
    auto height = 720;
    auto width = 1280;

    for (; i < argc; i++) {
        if (std::strcmp(argv[i], "--scene") == 0) {
            filename = argv[++i];
        } else if (std::strcmp(argv[i], "--height") == 0) {
            height = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "--width") == 0) {
            width = std::atoi(argv[++i]);
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized. " << SDL_GetError() << std::endl;
        return 0;
    }

    g_Window = std::shared_ptr<Window>(new Window(height, width));

    auto aspectRatio = width / static_cast<float>(height);

    g_Camera = std::shared_ptr<Camera>(new Camera(aspectRatio, 78.f, 1.f, 100000.f, glm::vec3(-200.f, 200.f, 0.f)));
    g_Control = std::shared_ptr<Control>(new Control());
    g_LightEnvironment = std::shared_ptr<LightEnvironment>(new LightEnvironment(glm::vec3(0.05f), glm::vec3(1.f), glm::vec3(0.545847f, -0.823136f, 0.156519f)));
    g_Render = std::shared_ptr<Render>(new Render());
    g_Ui = std::shared_ptr<Ui>(new Ui());

    auto clock = std::chrono::system_clock::now();
    auto events = std::vector<SDL_Event>();
    auto quit = false;

    g_Render->LoadScene(Scene(g_ResourcePath / std::filesystem::path(filename)));

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
        g_Render->Update();
        g_Ui->Update(events);
        g_Window->Update();    
    }

    g_Ui = nullptr;
    g_Render = nullptr;
    g_Window = nullptr;
    return 0;
}
