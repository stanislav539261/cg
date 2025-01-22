#include <cstring>
#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "image.hpp"

Image::Image(const std::filesystem::path &filename) {
    auto channels = 0;
    auto height = 0;
    auto width = 0;
    auto data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (data) {
        std::cout << "Load texture: " << filename << std::endl;

        m_Channels = channels;
        m_Height = height;
        m_Width = width;
        m_Data = static_cast<void *>(data);
    } else {
        std::cout << "Can't load texture: " << filename << std::endl;

        m_Channels = 0;
        m_Height = 0;
        m_Width = 0;
        m_Data = nullptr;
    }
}

Image::~Image() {
    stbi_image_free(m_Data);
}

unsigned int Image::MipLevel() const {
    auto minHeight = m_Height;
    auto minWidth = m_Width;
    auto mipLevel = 0u;

    while (minHeight != 0 && minWidth != 0) {
        minWidth /= 2;
        minHeight /= 2;
        mipLevel++;
    }

    return mipLevel;
}

unsigned int Image::Size() const {
    return m_Width * m_Height * m_Channels;
}