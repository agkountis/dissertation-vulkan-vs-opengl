#include "vulkan_pipeline_cache.h"
#include "logger.h"
#include "vulkan_infrastructure_context.h"

VulkanPipelineCache::~VulkanPipelineCache()
{
	vkDestroyPipelineCache(G_VulkanDevice, m_PipelineCache, nullptr);
}

bool VulkanPipelineCache::Create() noexcept
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VkResult result{ vkCreatePipelineCache(G_VulkanDevice, &pipelineCacheCreateInfo, nullptr, &m_PipelineCache) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline cache.");
		return false;
	}

	return true;
}

VulkanPipelineCache::operator VkPipelineCache() const noexcept
{
	return m_PipelineCache;
}
