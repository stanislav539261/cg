#include <memory>
#include <vector>

#include <SDL2/SDL_events.h>

class Control {
public:
    Control();
    ~Control();

    void                        Update(const std::vector<SDL_Event> &);
};

extern std::shared_ptr<Control> g_Control; 