#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include "light.hpp"
#include "render.hpp"
#include "ui.hpp"
#include "window.hpp"

std::shared_ptr<Ui> g_Ui = nullptr;

Ui::Ui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (g_Render && g_Render->m_Context && g_Window && g_Window->m_Window) {
        ImGui_ImplSDL2_InitForOpenGL(g_Window->m_Window, g_Render->m_Context);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    m_ShowMenu = false;
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

        auto &io = ImGui::GetIO();

        if (m_ShowMenu) {
            io.MouseDrawCursor = true;

            ImGui::Begin("Menu");

            ImGui::Checkbox("Enable Ambient Occlusion", &g_Render->m_EnableAmbientOcclusion);
            ImGui::Checkbox("Enable Reverse Z", &g_Render->m_EnableReverseZ);

            ImGui::SeparatorText("LightEnvironment");
            ImGui::SliderFloat3("Ambient color", reinterpret_cast<float *>(&g_LightEnvironment->m_AmbientColor), 0.f, 1.f);
            ImGui::SliderFloat3("Base color", reinterpret_cast<float *>(&g_LightEnvironment->m_BaseColor), 0.f, 1.f);
            ImGui::SliderFloat("Pitch", &g_LightEnvironment->m_Pitch, 0.f, 360.f);
            ImGui::SliderFloat("Yaw", &g_LightEnvironment->m_Yaw, 0.f, 360.f);

            ImGui::End();
        } else {
            io.MouseDrawCursor = false;
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}