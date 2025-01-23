#include "imgui.h"
#include "render.hpp"
#include "menu.hpp"

Menu::Menu() {
    m_ShowAnotherWindow = true;
    m_ShowDemoWindow = true;
}

Menu::~Menu() {

}

void Menu::Show() {
    ImGui::Begin("Menu");
    ImGui::Checkbox("Enable Reverse Z", &g_Render->m_EnableReverseZ);
    ImGui::End();
}