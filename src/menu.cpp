#include "imgui.h"

#include "light.hpp"
#include "render.hpp"
#include "menu.hpp"

Menu::Menu() {
    
}

Menu::~Menu() {

}

void Menu::Show() {
    ImGui::Begin("Menu");
    ImGui::Checkbox("Enable Ambient Occlusion", &g_Render->m_EnableAmbientOcclusion);
    ImGui::Checkbox("Enable Reverse Z", &g_Render->m_EnableReverseZ);
    ImGui::SeparatorText("LightEnvironment");
    ImGui::SliderFloat3("Ambient color", reinterpret_cast<float *>(&g_LightEnvironment->m_AmbientColor), 0.f, 1.f);
    ImGui::SliderFloat3("Base color", reinterpret_cast<float *>(&g_LightEnvironment->m_BaseColor), 0.f, 1.f);
    ImGui::SliderFloat("Pitch", &g_LightEnvironment->m_Pitch, 0.f, 360.f);
    ImGui::SliderFloat("Yaw", &g_LightEnvironment->m_Yaw, 0.f, 360.f);
    ImGui::End();
}