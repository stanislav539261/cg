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
    ImGui::Checkbox("Enable Reverse Z", &g_Render->m_EnableReverseZ);
    ImGui::SliderFloat("LightEnvironment Pitch", &g_LightEnvironment->m_Pitch, 0.f, 360.f);
    ImGui::SliderFloat("LightEnvironment Yaw", &g_LightEnvironment->m_Yaw, 0.f, 360.f);
    ImGui::End();
}