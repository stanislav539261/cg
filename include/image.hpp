#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <filesystem>

#include <glm/glm.hpp>

class Image {
public:
    Image(const std::filesystem::path &);
    ~Image();

    unsigned int    MipLevel() const;
    unsigned int    Size() const;
    
    unsigned int    m_Channels;
    unsigned int    m_Height;
    unsigned int    m_Width;
    void *          m_Data;
};

#endif /* IMAGE_HPP */