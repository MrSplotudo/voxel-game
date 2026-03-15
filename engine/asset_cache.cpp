#include "asset_cache.h"
#include "vulkan_context.h"
#include "vulkan_pipeline.h"
#include "vulkan_texture.h"

AssetCache::AssetCache(VulkanContext* contextIn, VulkanPipeline* pipelineIn) : context(contextIn), pipeline(pipelineIn) {
}

AssetCache::~AssetCache() {
    for (auto& mesh : meshCache) {
        delete mesh.second;
    }
    for (auto& index : indexCache) {
        delete index.second;
    }
    for (auto& texture : textureCache) {
        delete texture.second;
    }
}

MeshBuffers AssetCache::getMesh(const std::string& path) {
    MeshBuffers meshBuffer;

    if (meshCache.contains(path)) {

        meshBuffer.vertexBuffer = meshCache[path];
        meshBuffer.indexBuffer = indexCache[path];

        return meshBuffer;
    }

    auto [mesh, index] = loadMesh("../" + path, context->getDevice(), context->getPhysicalDevice());
    meshCache[path] = mesh;
    indexCache[path] = index;

    meshBuffer.vertexBuffer = meshCache[path];
    meshBuffer.indexBuffer = indexCache[path];

    return meshBuffer;
}

VulkanTexture* AssetCache::getTexture(const std::string& path) {
    if (textureCache.contains(path)) {
        return textureCache[path];
    }
    VulkanTexture* texture = new VulkanTexture(
        context->getDevice(),
        context->getPhysicalDevice(),
        context->getGraphicsQueue(),
        context->findQueueFamilies(context->getPhysicalDevice()).graphicsFamily);
    texture->load("../" + path, pipeline->getDescriptorSetLayout());
    textureCache[path] = texture;

    return texture;
}
