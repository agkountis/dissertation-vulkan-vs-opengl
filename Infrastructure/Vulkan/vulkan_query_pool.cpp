#include "vulkan_query_pool.h"

VulkanQueryPool::~VulkanQueryPool()
{
	vkDestroyQueryPool(G_VulkanDevice, m_QueryPool, nullptr);
}

bool VulkanQueryPool::Initialize(VkQueryType queryType,
                                 ui32 queryCount,
                                 VkQueryPipelineStatisticFlags pipelineStatisticFlags) noexcept
{
	if (queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
		assert(pipelineStatisticFlags != VK_NULL_HANDLE);
	}

	assert(queryCount);

	VkQueryPoolCreateInfo queryPoolCreateInfo{};
	queryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolCreateInfo.queryType = queryType;
	queryPoolCreateInfo.queryCount = queryCount;
	queryPoolCreateInfo.pipelineStatistics = pipelineStatisticFlags;

	VkResult result{ vkCreateQueryPool(G_VulkanDevice,
	                                   &queryPoolCreateInfo,
	                                   nullptr,
	                                   &m_QueryPool) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create query pool.");
		return false;
	}

	m_QueryType = queryType;
	m_QueryCount = queryCount;

	return true;
}

VulkanQueryPool::operator VkQueryPool() noexcept
{
	return m_QueryPool;
}

