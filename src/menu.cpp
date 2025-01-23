#include "imgui.h"
#include "render.hpp"
#include "menu.hpp"

Menu::Menu() {
    
}

Menu::~Menu() {

}

void Menu::Show() {
    ImGui::Begin("Menu");
    ImGui::Checkbox("Enable Reverse Z", &g_Render->m_EnableReverseZ);
    ImGui::End();
}