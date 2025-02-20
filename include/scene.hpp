#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <vector>

#include "object.hpp"

class Scene {
public:
    Scene();
    ~Scene();

    Object *                                Get(Handle);
    Handle                                  Insert(std::unique_ptr<Object> &&);
    bool                                    Remove(Handle);
    void                                    Update();
    
private:
    std::vector<std::unique_ptr<Object>>    m_Objects;
};

extern std::unique_ptr<Scene> g_Scene;

#endif /* SCENE_HPP */