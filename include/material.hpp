#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "image.hpp"

struct Material {
    std::shared_ptr<Image> m_DiffuseImage;
    std::shared_ptr<Image> m_MetalnessImage;
    std::shared_ptr<Image> m_NormalImage;
    std::shared_ptr<Image> m_RoughnessImage;
};

#endif /* MATERIAL_HPP */