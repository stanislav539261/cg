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

            // Global
            ImGui::Checkbox("Enable Ambient Occlusion", &g_Render->m_EnableAmbientOcclusion);
            ImGui::Checkbox("Enable Reverse Z", &g_Render->m_EnableReverseZ);
            ImGui::Checkbox("Enable Wireframe Mode", &g_Render->m_EnableWireframeMode);

            // LightEnvironment
            ImGui::SeparatorText("LightEnvironment");
            ImGui::SliderFloat3("Ambient color", reinterpret_cast<float *>(&g_LightEnvironment->m_AmbientColor), 0.f, 1.f);
            ImGui::SliderFloat3("Base color", reinterpret_cast<float *>(&g_LightEnvironment->m_BaseColor), 0.f, 1.f);
            ImGui::SliderFloat("Pitch", &g_LightEnvironment->m_Pitch, 0.f, 360.f);
            ImGui::SliderFloat("Yaw", &g_LightEnvironment->m_Yaw, 0.f, 360.f);

            // LightPoints
            auto i = 0u;

            for (; i < g_LightPoints.size(); i++) {
                const auto lightPointName = std::string("LightPoint ") + std::to_string(i);

                ImGui::SeparatorText(lightPointName.c_str());

                auto &lightPoint = g_LightPoints[i];

                const auto positonName = std::string("Position##LightPoint") + std::to_string(i);
                const auto baseColorName = std::string("Base color##LightPoint") + std::to_string(i);
                const auto radiusName = std::string("Radius##LightPoint") + std::to_string(i);

                ImGui::DragFloat3(positonName.c_str(), reinterpret_cast<float *>(&lightPoint->m_Position));
                ImGui::SliderFloat3(baseColorName.c_str(), reinterpret_cast<float *>(&lightPoint->m_BaseColor), 0.f, 1.f);
                ImGui::DragFloat(radiusName.c_str(), reinterpret_cast<float *>(&lightPoint->m_Radius));
            }

            const auto lastLightPointName = std::string("LightPoint ") + std::to_string(i);

            ImGui::SeparatorText(lastLightPointName.c_str());
            
            if (ImGui::Button("Add source")) {
                const auto lightPoint = std::make_shared<LightPoint>(g_Camera->m_Position, glm::vec3(1.f), 800.f);

                g_LightPoints.push_back(lightPoint);
            };

            // Shadows
            ImGui::SeparatorText("Shadows");
            ImGui::SliderFloat("CSM filter radius", &g_Render->m_ShadowCsmFilterRadius, 0.f, 16.f);
            ImGui::SliderFloat("Cube filter radius", &g_Render->m_ShadowCubeFilterRadius, 0.f, 16.f);

            ImGui::End();
        } else {
            io.MouseDrawCursor = false;
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}