#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include "render.hpp"
#include "ui.hpp"
#include "window.hpp"

std::shared_ptr<Ui> g_Ui = nullptr;

Ui::Ui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.MouseDrawCursor = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (g_Render && g_Render->m_Context && g_Window && g_Window->m_Window) {
        ImGui_ImplSDL2_InitForOpenGL(g_Window->m_Window, g_Render->m_Context);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    m_Menu = std::shared_ptr<Menu>(new Menu());
}

Ui::~Ui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Ui::Update(const std::vector<SDL_Event> &events) {
    for (const auto &event: events) {
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    if (g_Render && g_Render->m_Context && g_Window && g_Window->m_Window) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        m_Menu->Show();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}