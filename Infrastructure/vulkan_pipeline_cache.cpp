#include "vulkan_pipeline_cache.h"
#include "logger.h"

VulkanPipelineCache::~VulkanPipelineCache()
{
	vkDestroyPipelineCache(m_pLogicalDevice, m_PipelineCache, nullptr);
}

bool VulkanPipelineCache::Create(VkDevice logicalDevice) noexcept
{
	m_pLogicalDevice = logicalDevice;

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VkResult result{ vkCreatePipelineCache(m_pLogicalDevice, &pipelineCacheCreateInfo, nullptr, &m_PipelineCache) };

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
