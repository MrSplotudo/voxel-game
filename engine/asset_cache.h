#pragma once
#include "vulkan_buffer.h"
#include <string>
#include <unordered_map>

class VulkanContext;
class VulkanPipeline;
class VulkanBuffer;
class VulkanTexture;

class AssetCache {
public:
    AssetCache(VulkanContext* contextIn, VulkanPipeline* pipelineIn);
    ~AssetCache();

    MeshBuffers getMesh(const std::string& path);
    VulkanTexture* getTexture(const std::string& path);


private:
    std::unordered_map<std::string, VulkanBuffer*> meshCache;
    std::unordered_map<std::string, VulkanBuffer*> indexCache;
    std::unordered_map<std::string, VulkanTexture*> textureCache;

    VulkanContext* context;
    VulkanPipeline* pipeline;
};