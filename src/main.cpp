#include <cstring>
#include <iostream>

#include <SDL2/SDL.h>

#include "camera.hpp"
#include "control.hpp"
#include "model.hpp"
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
        if (std::strcmp(argv[i], "--model") == 0) {
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

    g_Window = std::make_shared<Window>(height, width);

    auto aspectRatio = width / static_cast<float>(height);

    g_Control = std::make_shared<Control>();
    g_Render = std::make_unique<Render>();
    g_Scene = std::make_unique<Scene>();
    g_Ui = std::make_shared<Ui>();

    auto clock = std::chrono::system_clock::now();
    auto events = std::vector<SDL_Event>();
    auto quit = false;
    

    g_Render->LoadModel(Model(g_ResourcePath / std::filesystem::path(filename)));

    // Initialize scene
    auto camera = std::make_unique<Camera>();
    camera->m_Position = glm::vec3(0.f, 200.f, 0.f);

    auto lightEnvironment = std::make_unique<LightEnvironment>();
    lightEnvironment->m_Angles = glm::vec3(260.f, 20.f, 0.f);
    lightEnvironment->m_AmbientColor = glm::vec3(0.2f);
    lightEnvironment->m_BaseColor = glm::vec3(1.f);

    g_Scene->Insert(std::move(camera));
    g_Scene->Insert(std::move(lightEnvironment));

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

        g_Scene->Update();
        g_Control->Update(events);
        g_Render->Update();
        g_Ui->Update(events);
        g_Window->Update();    
    }

    g_Scene = nullptr;
    g_Ui = nullptr;
    g_Render = nullptr;
    g_Window = nullptr;
    return 0;
}
