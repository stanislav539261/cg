#include "scene.hpp"

std::unique_ptr<Scene> g_Scene = nullptr;

Scene::Scene() {
    m_Objects = std::vector<std::unique_ptr<Object>>();
}

Scene::~Scene() {

}

Object *Scene::Get(Handle handle) {
    auto object = static_cast<Object *>(nullptr);

    if (handle < m_Objects.size()) {
        if (const auto &obj = m_Objects[handle]) {
            object = obj.get();
        }
    }

    return object;
}

Handle Scene::Insert(std::unique_ptr<Object> &&object) {
    const auto handle = m_Objects.size();

    object->m_Handle = handle;

    m_Objects.push_back(std::move(object));

    return handle;
}

bool Scene::Remove(Handle handle) {
    auto isRemoved = false;

    if (handle < m_Objects.size()) {
        m_Objects[handle] = nullptr;
    }

    return isRemoved;
}

void Scene::Update() {
    for (const auto &object : m_Objects) {
        if (const auto &obj = object) {
            obj->Update();
        }
    }
}