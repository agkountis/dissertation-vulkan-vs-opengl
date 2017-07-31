#include "vulkan_command_pool.h"
#include "logger.h"
#include "vulkan_infrastructure_context.h"


VulkanCommandPool::~VulkanCommandPool()
{
	LOG("Cleaning up VulkanCommandPool");
	vkDestroyCommandPool(G_VulkanDevice, m_CommandPool, nullptr);
}

bool VulkanCommandPool::Create(ui32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags) noexcept
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndex;
	createInfo.flags = createFlags;
	VkResult result{ vkCreateCommandPool(G_VulkanDevice, &createInfo, nullptr, &m_CommandPool) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create command pool.");
		return false;
	}

	return true;
}

VulkanCommandPool::operator VkCommandPool() const noexcept
{
	return m_CommandPool;
}

VkCommandPool* VulkanCommandPool::operator&() noexcept
{
	return &m_CommandPool;
}

const VkCommandPool* VulkanCommandPool::operator&() const noexcept
{
	return &m_CommandPool;
}
