#include "vulkan_command_pool.h"
#include "../logger.h"


VulkanCommandPool::~VulkanCommandPool()
{
	vkDestroyCommandPool(m_pLogicalDevice, m_CommandPool, nullptr);
}

bool VulkanCommandPool::Create(VkDevice logicalDevice, ui32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags) noexcept
{
	m_pLogicalDevice = logicalDevice;

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndex;
	createInfo.flags = createFlags;
	VkResult result{ vkCreateCommandPool(m_pLogicalDevice, &createInfo, nullptr, &m_CommandPool) };

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
