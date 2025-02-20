#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>

#include <glm/glm.hpp>

#include "handle.hpp"

class Object {
public:
    virtual ~Object() = default;

    virtual bool    IsCamera() const = 0;
    virtual bool    IsLightEnvironment() const = 0;
    virtual bool    IsLightPoint() const = 0;
    virtual void    Update() = 0;

    Handle          m_Handle;
    std::string     m_Name;
    glm::vec3       m_Position;

protected:
    Object() = default;
};

#endif /* OBJECT_HPP */