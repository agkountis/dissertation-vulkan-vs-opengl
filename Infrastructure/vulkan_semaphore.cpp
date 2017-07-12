#include "vulkan_semaphore.h"
#include "logger.h"

VulkanSemaphore::~VulkanSemaphore()
{
	vkDestroySemaphore(m_pLogicalDevice, m_Semaphore, nullptr);
}

bool VulkanSemaphore::Create(VkDevice logicalDevice) noexcept
{
	m_pLogicalDevice = logicalDevice;

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result{ vkCreateSemaphore(m_pLogicalDevice, &semaphoreCreateInfo, nullptr, &m_Semaphore) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create semaphore");
		return false;
	}

	return true;
}

VulkanSemaphore::operator VkSemaphore() const noexcept
{
	return m_Semaphore;
}

const VkSemaphore* VulkanSemaphore::operator&() const noexcept
{
	return &m_Semaphore;
}