#ifndef MENU_HPP
#define MENU_HPP

class Menu {
public:
    Menu();
    ~Menu();

    void        Show();

    bool        m_ShowAnotherWindow;
    bool        m_ShowDemoWindow;
};

#endif /* MENU_HPP */