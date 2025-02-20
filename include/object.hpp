#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

#include "handle.hpp"

class Object {
public:
    virtual ~Object() = default;

    glm::vec3       Forward() const;
    virtual bool    IsCamera() const = 0;
    virtual bool    IsLightEnvironment() const = 0;
    virtual bool    IsLightPoint() const = 0;
    glm::quat       Rotation() const;
    void            Translate(const glm::vec3 &);
    virtual void    Update() = 0;

    glm::vec3       m_Angles;
    Handle          m_Handle;
    std::string     m_Name;
    glm::vec3       m_Position;

protected:
    Object() = default;
};

#endif /* OBJECT_HPP */