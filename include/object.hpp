#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>

#include <glm/glm.hpp>

#include "handle.hpp"

class Object {
public:
    virtual ~Object() = default;

    virtual bool    IsCamera() const { return false; }
    virtual bool    IsLightEnvironment() const { return false; }
    virtual bool    IsLightPoint() const { return false; }
    virtual void    Update() {};

    Handle          m_Handle;
    std::string     m_Name;
    glm::vec3       m_Position;

protected:
    Object() = default;
};

#endif /* OBJECT_HPP */