#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"
#include "vulkan_texture.h"
#include <iostream>
#include <string>

VulkanTexture::VulkanTexture(VkDevice deviceIn, VkPhysicalDevice physicalDeviceIn) : device(deviceIn), physicalDevice(physicalDeviceIn){
}

VulkanTexture::~VulkanTexture() {
}

void VulkanTexture::load(const std::string& filePath) {
    int width, height, channels;
    unsigned char* pixelData = stbi_load(filePath.c_str(), &width, &height, &channels, 4);

    if (pixelData == nullptr) {
        std::cout << "Failed to load texture: " << filePath << std::endl;
        return;
    }

    std::cout << "Loaded image: " << width << "*" << height << std::endl;

    stbi_image_free(pixelData);
}
